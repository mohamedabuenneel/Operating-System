/*
 * fault_handler.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include "trap.h"
#include <kern/proc/user_environment.h>
#include "../cpu/sched.h"
#include "../disk/pagefile_manager.h"
#include "../mem/memory_manager.h"

//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;

//===============================
// REPLACEMENT STRATEGIES
//===============================
//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE)
{
	assert(LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
	_PageRepAlgoType = LRU_TYPE ;
}
void setPageReplacmentAlgorithmCLOCK(){_PageRepAlgoType = PG_REP_CLOCK;}
void setPageReplacmentAlgorithmFIFO(){_PageRepAlgoType = PG_REP_FIFO;}
void setPageReplacmentAlgorithmModifiedCLOCK(){_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;}
/*2018*/ void setPageReplacmentAlgorithmDynamicLocal(){_PageRepAlgoType = PG_REP_DYNAMIC_LOCAL;}
/*2021*/ void setPageReplacmentAlgorithmNchanceCLOCK(int PageWSMaxSweeps){_PageRepAlgoType = PG_REP_NchanceCLOCK;  page_WS_max_sweeps = PageWSMaxSweeps;}

//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE){return _PageRepAlgoType == LRU_TYPE ? 1 : 0;}
uint32 isPageReplacmentAlgorithmCLOCK(){if(_PageRepAlgoType == PG_REP_CLOCK) return 1; return 0;}
uint32 isPageReplacmentAlgorithmFIFO(){if(_PageRepAlgoType == PG_REP_FIFO) return 1; return 0;}
uint32 isPageReplacmentAlgorithmModifiedCLOCK(){if(_PageRepAlgoType == PG_REP_MODIFIEDCLOCK) return 1; return 0;}
/*2018*/ uint32 isPageReplacmentAlgorithmDynamicLocal(){if(_PageRepAlgoType == PG_REP_DYNAMIC_LOCAL) return 1; return 0;}
/*2021*/ uint32 isPageReplacmentAlgorithmNchanceCLOCK(){if(_PageRepAlgoType == PG_REP_NchanceCLOCK) return 1; return 0;}

//===============================
// PAGE BUFFERING
//===============================
void enableModifiedBuffer(uint32 enableIt){_EnableModifiedBuffer = enableIt;}
uint8 isModifiedBufferEnabled(){  return _EnableModifiedBuffer ; }

void enableBuffering(uint32 enableIt){_EnableBuffering = enableIt;}
uint8 isBufferingEnabled(){  return _EnableBuffering ; }

void setModifiedBufferLength(uint32 length) { _ModifiedBufferLength = length;}
uint32 getModifiedBufferLength() { return _ModifiedBufferLength;}

//===============================
// FAULT HANDLERS
//===============================

//Handle the table fault
void table_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
#if USE_KHEAP
	{
		ptr_table = create_page_table(curenv->env_page_directory, (uint32)fault_va);
	}
#else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}
#endif
}

//Handle the page fault

void page_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//TODO: [PROJECT MS3] [FAULT HANDLER] page_fault_handler
	// Write your code here, remove the panic and write your code
	//panic("page_fault_handler() is not implemented yet...!!");
	uint32 Mysize=env_page_ws_get_size(curenv);

	void placement(struct Env * curenv,uint32 fault_va)
	{
		allocate_chunk(curenv->env_page_directory,ROUNDDOWN(fault_va,PAGE_SIZE),PAGE_SIZE,PERM_USER|PERM_WRITEABLE);
		int x=pf_read_env_page(curenv,(void*)fault_va);

		for( int i =0 ; i<curenv->page_WS_max_size;i++)
			{
				if(curenv->ptr_pageWorkingSet[curenv->page_last_WS_index].empty)
					break;
				else if(curenv->ptr_pageWorkingSet[i].empty)
				{
					curenv->page_last_WS_index = i ;
					break ;
				}
			}

		if(x==0)
		{


			env_page_ws_set_entry(curenv,curenv->page_last_WS_index,fault_va);
			curenv->page_last_WS_index=curenv->page_last_WS_index+1;
			if(curenv->page_last_WS_index==curenv->page_WS_max_size)
							{
								curenv->page_last_WS_index=0;
							}



		}

		if(x==E_PAGE_NOT_EXIST_IN_PF)
		{


			if((fault_va>=USER_HEAP_START)&&(fault_va<USER_HEAP_MAX))
			{
				env_page_ws_set_entry(curenv,curenv->page_last_WS_index,fault_va);
				curenv->page_last_WS_index=curenv->page_last_WS_index+1;
				if(curenv->page_last_WS_index==curenv->page_WS_max_size)
				{
					curenv->page_last_WS_index=0;
				}



			}
			else if((fault_va>=USER_HEAP_MAX)&&(fault_va<USTACKTOP))
					{
						env_page_ws_set_entry(curenv,curenv->page_last_WS_index,fault_va);
						curenv->page_last_WS_index=curenv->page_last_WS_index+1;
						if(curenv->page_last_WS_index==curenv->page_WS_max_size)
										{
											curenv->page_last_WS_index=0;
										}



					}
			else
				panic("ILLEGAL MEMORY ACCESS ");





		}


	}
	// Hanshta8l Placement
	if(curenv->page_WS_max_size>Mysize)
	{
		placement(curenv,fault_va);

	}



	//Hanshta8l replacemnt
	if(curenv->page_WS_max_size==Mysize)
	{
		uint32 va;
		uint32 Myperm;
		uint32 last_index=curenv->page_last_WS_index;

		uint32 i =last_index;

		while(1)
		{
			if(i==(curenv->page_WS_max_size))
			{
				i=0;
				curenv->page_last_WS_index=0;
			}


	      va=(curenv->ptr_pageWorkingSet[i]).virtual_address;
		  Myperm=pt_get_page_permissions(curenv->env_page_directory,va);

		if((Myperm&PERM_USED)==PERM_USED)
		{
			uint32 *pageptr;
			get_page_table(curenv->env_page_directory,va,&pageptr);
			pageptr[PTX(va)]=pageptr[PTX(va)] & (~PERM_USED);
			curenv->page_last_WS_index=curenv->page_last_WS_index+1;
			i+=1;
			if(i==(curenv->page_WS_max_size))
					{
						i=0;
						curenv->page_last_WS_index=0;
					}



		}

//de law el used bit b 0 ya3ny dah hayba el viktim
		else
		{
			//hena batcheck 3ala el modfied bit ba3d ma 3erft el viktim


			if((Myperm&PERM_MODIFIED)==PERM_MODIFIED)
			{
				struct FrameInfo* p=NULL;
				uint32* pt;
				p=get_frame_info(curenv->env_page_directory,va,&pt);
				pf_update_env_page(curenv,va,p);

			}


			env_page_ws_invalidate(curenv,va);
			unmap_frame(curenv->env_page_directory,va);
			placement(curenv,fault_va);
			break;




		}




			}








	}






	//refer to the project presentation and documentation for details
}
void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
{
	// Write your code here, remove the panic and write your code
	panic("__page_fault_handler_with_buffering() is not implemented yet...!!");


}
