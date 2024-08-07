/*
 * dyn_block_management.c
 *
 *  Created on: Sep 21, 2022
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"
struct MemBlock *VarForNF=NULL;


//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//===========================
// PRINT MEM BLOCK LISTS:
//===========================

void print_mem_block_lists()
{
	cprintf("\n=========================================\n");
	struct MemBlock* blk ;
	struct MemBlock* lastBlk = NULL ;
	cprintf("\nFreeMemBlocksList:\n");
	uint8 sorted = 1 ;
	LIST_FOREACH(blk, &FreeMemBlocksList)
	{
		if (lastBlk && blk->sva < lastBlk->sva + lastBlk->size)
			sorted = 0 ;
		cprintf("[%x, %x)-->", blk->sva, blk->sva + blk->size) ;
		lastBlk = blk;
	}
	if (!sorted)	cprintf("\nFreeMemBlocksList is NOT SORTED!!\n") ;

	lastBlk = NULL ;
	cprintf("\nAllocMemBlocksList:\n");
	sorted = 1 ;
	LIST_FOREACH(blk, &AllocMemBlocksList)
	{
		if (lastBlk && blk->sva < lastBlk->sva + lastBlk->size)
			sorted = 0 ;
		cprintf("[%x, %x)-->", blk->sva, blk->sva + blk->size) ;
		lastBlk = blk;
	}
	if (!sorted)	cprintf("\nAllocMemBlocksList is NOT SORTED!!\n") ;
	cprintf("\n=========================================\n");

}

//********************************************************************************//
//********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//===============================
// [1] INITIALIZE AVAILABLE LIST:
//===============================
void initialize_MemBlocksList(uint32 numOfBlocks)
{
	  LIST_INIT(&AvailableMemBlocksList);
	   for(int i=0;i<numOfBlocks;i++)
	   {

		   LIST_INSERT_TAIL(&AvailableMemBlocksList,&MemBlockNodes[i]);
	   }

}

//===============================
// [2] FIND BLOCK:
//===============================
struct MemBlock *find_block(struct MemBlock_List *blockList, uint32 va)
{
	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] find_block
		// Write your code here, remove the panic and write your code
		struct MemBlock *ptrFind;

		 LIST_FOREACH(ptrFind,blockList){

			 if(ptrFind->sva==va){
				 return ptrFind;
			 }
		 }
		  return NULL;
}





//=========================================
// [3] INSERT BLOCK IN ALLOC LIST [SORTED]:
//=========================================
void insert_sorted_allocList(struct MemBlock *blockToInsert)
{

		struct MemBlock *p;
			struct MemBlock *Nextp;

			if(LIST_EMPTY(&AllocMemBlocksList))
			{
				LIST_INSERT_HEAD(&AllocMemBlocksList,blockToInsert);
			}

			else if  (blockToInsert->sva>AllocMemBlocksList.lh_last->sva)
			{
				LIST_INSERT_TAIL(&AllocMemBlocksList,blockToInsert);
			}
			else if (blockToInsert->sva<AllocMemBlocksList.lh_first->sva)
			{
				LIST_INSERT_HEAD(&AllocMemBlocksList,blockToInsert);

			}
			else
			{
				LIST_FOREACH(p,&AllocMemBlocksList)
				{
					Nextp=LIST_NEXT(p);
					if((blockToInsert->sva>p->sva) && (blockToInsert->sva<Nextp->sva))
					{
						LIST_INSERT_AFTER(&AllocMemBlocksList,p,blockToInsert);
						break;

					}

				}


			}



}





//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
struct MemBlock *alloc_block_FF(uint32 size)
{
	struct MemBlock *Ite2;
		struct MemBlock *TheNew;


			LIST_FOREACH(Ite2,&FreeMemBlocksList)
			{
				if(Ite2->size==size)
				{
					LIST_REMOVE(&FreeMemBlocksList,Ite2);
					VarForNF=Ite2;
					return Ite2;

				}
				if(Ite2->size>size)

			    {
					Ite2->size=(Ite2->size)-size;
					TheNew=LIST_FIRST(&AvailableMemBlocksList);
					LIST_REMOVE(&AvailableMemBlocksList,TheNew);
					TheNew->size=size;
					TheNew->sva=Ite2->sva;
					Ite2->sva=(Ite2->sva)+size;
					VarForNF=TheNew;
					return TheNew;

			   }
			}
			VarForNF=NULL;
			return NULL;

}

//=========================================
// [5] ALLOCATE BLOCK BY BEST FIT:
//=========================================
struct MemBlock *alloc_block_BF(uint32 size)
{
	struct MemBlock *i;
		struct MemBlock *TheLowest=NULL;
		struct MemBlock *TheNew1;

		LIST_FOREACH(i,&FreeMemBlocksList)
		{
			if(i->size==size)
			{
				LIST_REMOVE(&FreeMemBlocksList,i);
				VarForNF=i;
				return i;
			}
		}

		LIST_FOREACH(i,&FreeMemBlocksList)
		{

			if(i->size>size)
			{
				if(TheLowest==NULL)
				{
					TheLowest=i;
				}


			    if(TheLowest->size>i->size &&i->size>size )
			 	     TheLowest=i;








		   }
	   }
		if(TheLowest==NULL){
			VarForNF=NULL;
			return NULL;}
		else
		{
		TheLowest->size=(TheLowest->size)-size;
		TheNew1=LIST_FIRST(&AvailableMemBlocksList);
		LIST_REMOVE(&AvailableMemBlocksList,TheNew1);
		TheNew1->size=size;
		TheNew1->sva=TheLowest->sva;
		TheLowest->sva=(TheLowest->sva)+size;
		VarForNF=TheNew1;
		return TheNew1;
		}
}


//=========================================
// [7] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
struct MemBlock *alloc_block_NF(uint32 size)
{
	struct MemBlock *Ite2F;
	struct MemBlock *TheNewF;


	if(VarForNF==NULL)
      {
		LIST_FOREACH(Ite2F,&FreeMemBlocksList)
				{
					if(Ite2F->size==size)
					{
						LIST_REMOVE(&FreeMemBlocksList,Ite2F);
						VarForNF=Ite2F;
						return Ite2F;

					}
					if(Ite2F->size>size)

					{
						Ite2F->size=(Ite2F->size)-size;
						TheNewF=LIST_FIRST(&AvailableMemBlocksList);
						LIST_REMOVE(&AvailableMemBlocksList,TheNewF);
						TheNewF->size=size;
						TheNewF->sva=Ite2F->sva;
						Ite2F->sva=(Ite2F->sva)+size;
						VarForNF=TheNewF;
						return TheNewF;

				    }




			       }

		               VarForNF=NULL;
						return NULL;
	       }
	else {
		LIST_FOREACH(Ite2F,&FreeMemBlocksList)
					{
						if(Ite2F->size==size && Ite2F->sva>VarForNF->sva)
						{
							LIST_REMOVE(&FreeMemBlocksList,Ite2F);
							VarForNF=Ite2F;
							return Ite2F;

						}
						if(Ite2F->size>size && Ite2F->sva>VarForNF->sva)

					    {
							Ite2F->size=(Ite2F->size)-size;
							TheNewF=LIST_FIRST(&AvailableMemBlocksList);
							LIST_REMOVE(&AvailableMemBlocksList,TheNewF);
							TheNewF->size=size;
							TheNewF->sva=Ite2F->sva;
							Ite2F->sva=(Ite2F->sva)+size;
							VarForNF=TheNewF;
							return TheNewF;

					   }
					}

					// LAW 2ABLEH MAFESH AY ALLOCATION
					LIST_FOREACH(Ite2F,&FreeMemBlocksList)
								{
									if(Ite2F->size==size)
									{
										LIST_REMOVE(&FreeMemBlocksList,Ite2F);
										VarForNF=Ite2F;
										return Ite2F;

									}
									if(Ite2F->size>size)

								    {
										Ite2F->size=(Ite2F->size)-size;
										TheNewF=LIST_FIRST(&AvailableMemBlocksList);
										LIST_REMOVE(&AvailableMemBlocksList,TheNewF);
										TheNewF->size=size;
										TheNewF->sva=Ite2F->sva;
										Ite2F->sva=(Ite2F->sva)+size;
										VarForNF=TheNewF;
										return TheNewF;

								   }




		                      }

					VarForNF=NULL;
					return NULL;
	}



}

//===================================================
// [8] INSERT BLOCK (SORTED WITH MERGE) IN FREE LIST:
//===================================================
void insert_sorted_with_merge_freeList(struct MemBlock *blockToInsert)
{

	struct MemBlock *i;
		struct MemBlock *Nextpo;
		struct MemBlock *Prevpo;


			if(LIST_EMPTY(&FreeMemBlocksList))
			{
				LIST_INSERT_HEAD(&FreeMemBlocksList,blockToInsert);
			}
			else if  (blockToInsert->sva>FreeMemBlocksList.lh_last->sva)
			{

				//THE ALL CHANGES
				if((FreeMemBlocksList.lh_last->sva+FreeMemBlocksList.lh_last->size)==blockToInsert->sva)
				{
					FreeMemBlocksList.lh_last->size=FreeMemBlocksList.lh_last->size+blockToInsert->size;
					blockToInsert->size=0;
					blockToInsert->sva=0;
					LIST_INSERT_TAIL(&AvailableMemBlocksList,blockToInsert);
				}
				else
				{
					LIST_INSERT_TAIL(&FreeMemBlocksList,blockToInsert);
				}
				//to here
			}
			else if (blockToInsert->sva<FreeMemBlocksList.lh_first->sva)
			{

				//THE ALL CHANGES
				if((blockToInsert->sva+blockToInsert->size)==FreeMemBlocksList.lh_first->sva)
							{
					FreeMemBlocksList.lh_first->size=FreeMemBlocksList.lh_first->size+blockToInsert->size;
					FreeMemBlocksList.lh_first->sva=FreeMemBlocksList.lh_first->sva-blockToInsert->size;
								blockToInsert->size=0;
								blockToInsert->sva=0;
								LIST_INSERT_TAIL(&AvailableMemBlocksList,blockToInsert);
							}
				else
				{
				LIST_INSERT_HEAD(&FreeMemBlocksList,blockToInsert);
				}
				//to here

			}
			else
			{
				LIST_FOREACH(i,&FreeMemBlocksList)
				{
					Nextpo=LIST_NEXT(i);
					if(blockToInsert->sva>i->sva && blockToInsert->sva<Nextpo->sva)
					{
						//the all changes
					/*adjance le el ba3do we elly 2ablo*/ if((i->sva+i->size)==blockToInsert->sva&&blockToInsert->sva+blockToInsert->size==Nextpo->sva)
						{
						i->size=(i->size+blockToInsert->size+Nextpo->size);
						LIST_REMOVE(&FreeMemBlocksList,Nextpo);
						blockToInsert->size=0;
						blockToInsert->sva=0;
						Nextpo->size=0;
						Nextpo->sva=0;
						LIST_INSERT_TAIL(&AvailableMemBlocksList,blockToInsert);
						LIST_INSERT_TAIL(&AvailableMemBlocksList,Nextpo);
						break;





						}
					/*adjance le elly 2ablo*/ else if((i->sva+i->size)==blockToInsert->sva)
									{
						                i->size=i->size+blockToInsert->size;
										blockToInsert->size=0;
										blockToInsert->sva=0;
										LIST_INSERT_TAIL(&AvailableMemBlocksList,blockToInsert);
										break;
									}
					/*adjance le el ba3do*/	else if((blockToInsert->sva+blockToInsert->size)==Nextpo->sva)
												{
						                         Nextpo->size=Nextpo->size+blockToInsert->size;
						                         Nextpo->sva=Nextpo->sva-blockToInsert->size;
												 blockToInsert->size=0;
												 blockToInsert->sva=0;
												 LIST_INSERT_TAIL(&AvailableMemBlocksList,blockToInsert);
												 break;

												}
					else
					{
					LIST_INSERT_AFTER(&FreeMemBlocksList,i,blockToInsert);
						break;
					}


				}
			}


			}




}

