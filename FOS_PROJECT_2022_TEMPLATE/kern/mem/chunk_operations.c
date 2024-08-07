/*
 * chunk_operations.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include <kern/trap/fault_handler.h>
#include <kern/disk/pagefile_manager.h>
#include "kheap.h"
#include "memory_manager.h"


/******************************/
/*[1] RAM CHUNKS MANIPULATION */
/******************************/

//===============================
// 1) CUT-PASTE PAGES IN RAM:
//===============================
//This function should cut-paste the given number of pages from source_va to dest_va
//if the page table at any destination page in the range is not exist, it should create it
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int cut_paste_pages(uint32* page_directory, uint32 source_va, uint32 dest_va, uint32 num_of_pages)
{
	//TODO: [PROJECT MS2] [CHUNK OPERATIONS] cut_paste_pages
	// Write your code here, remove the panic and write your code
	//panic("cut_paste_pages() is not implemented yet...!!");
	struct FrameInfo* destframe;
	struct FrameInfo* sourceframe;
    uint32* destptr;
    uint32* sourceptr;
    uint32* pagetablesource;
    uint32* pagetabledest;
    uint32 allperm;
    ///////////////////////
    uint32 dest=dest_va;
    uint32 source=source_va;


	for(int i=0;i<num_of_pages;i++)
	{
		destframe=get_frame_info(page_directory,dest,&destptr);
		if(destframe!=0)
		{
			return -1;

		}

		dest+=PAGE_SIZE;

	}
	dest=dest_va;
    source=source_va;

	for(int i=0;i<num_of_pages;i++)
	{
		sourceframe=get_frame_info(page_directory,source,&sourceptr);
		get_page_table(page_directory,source,&pagetablesource);
		get_page_table(page_directory,dest,&pagetabledest);
		if(pagetabledest==NULL)
		{
			create_page_table(page_directory,dest);
		}
		allperm=pt_get_page_permissions(page_directory,source);
		allperm=allperm<<20;
		allperm=allperm>>20;
		map_frame(page_directory,sourceframe,dest,allperm|PERM_PRESENT);
		unmap_frame(page_directory,source);
		dest+=PAGE_SIZE;
		source+=PAGE_SIZE;

	}
	return 0;




}

//===============================
// 2) COPY-PASTE RANGE IN RAM:
//===============================
//This function should copy-paste the given size from source_va to dest_va
//if the page table at any destination page in the range is not exist, it should create it
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int copy_paste_chunk(uint32* page_directory, uint32 source_va, uint32 dest_va, uint32 size)
{
	//TODO: [PROJECT MS2] [CHUNK OPERATIONS] copy_paste_chunk
	// Write your code here, remove the panic and write your code
	//panic("copy_paste_chunk() is not implemented yet...!!");
	struct FrameInfo* destframe;
	struct FrameInfo* sourceframe;
	    uint32* destptr;
	    uint32* sourceptr;
	    uint32* pagetablesource;
	    uint32* pagetabledest;
	    uint32 allperm;
	    ///////////////////////
	    uint32 dest=dest_va;
	    uint32 source=source_va;
	    for(int i=0;i<size;i++)
	    	{
	    		destframe=get_frame_info(page_directory,(dest+i),&destptr);

	    		if(destframe!=0)
	    		{
	    			int existperm=pt_get_page_permissions(page_directory,(dest+i));
	    			if((existperm & PERM_WRITEABLE)==0)
	    			{
	    				return -1;
	    			}


	    		}

	    		i+=PAGE_SIZE;
	    	}


	    dest=dest_va;
	    source=source_va;



		for(int i=0;i<size;i++)
		{
			int check;
			uint32 x;

			destframe=get_frame_info(page_directory,(dest+i),&destptr);
			if(destframe==NULL)
			{
				uint32* y;
				get_page_table(page_directory,dest+i,&y);

				if(y==NULL)
				{
					create_page_table(page_directory,dest+i);
				}

				 check=allocate_frame(&destframe);
				 x= pt_get_page_permissions(page_directory,(source+i));
				 if(check!=E_NO_MEM)
				 			{

				 				map_frame(page_directory,destframe,(dest+i),((x&PERM_USER)|PERM_WRITEABLE));
				 			}


			}

			i+=PAGE_SIZE;
		}

		char *s =  (char *) source_va;
		char *d =  (char *) dest_va;
			 		for(int i=0;i<size;i++)
			 		{
			 			*d = *s;
			 			s++;
			 			d++;
			 		}

			return 0;


}

//===============================
// 3) SHARE RANGE IN RAM:
//===============================
//This function should share the given size from dest_va with the source_va
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int share_chunk(uint32* page_directory, uint32 source_va,uint32 dest_va, uint32 size, uint32 perms)
{
	//TODO: [PROJECT MS2] [CHUNK OPERATIONS] share_chunk
	// Write your code here, remove the panic and write your code
	//panic("share_chunk() is not implemented yet...!!");
	struct FrameInfo* destframe;
		struct FrameInfo* sourceframe;
	    uint32* destptr;
	    uint32* sourceptr;
	    uint32* pagetabledest;
	    ///////////////////////


	   uint32 NewStartSource= ROUNDDOWN(source_va,PAGE_SIZE);
	   uint32 NewEndSource=ROUNDUP(source_va+size,PAGE_SIZE);
	   uint32 NewStartDest= ROUNDDOWN(dest_va,PAGE_SIZE);
	   uint32 NewEndDest= ROUNDUP(dest_va+size,PAGE_SIZE);


		for(int i=NewStartDest;i<NewEndDest;i+=PAGE_SIZE)
		{
			destframe=get_frame_info(page_directory,(NewStartDest),&destptr);
			if(destframe!=0)
			{
				return -1;

			}

			NewStartDest+=PAGE_SIZE;

		}

		 NewStartSource= ROUNDDOWN(source_va,PAGE_SIZE);
	     NewEndSource=ROUNDUP(source_va+size,PAGE_SIZE);
	     NewStartDest= ROUNDDOWN(dest_va,PAGE_SIZE);

		for(int i=NewStartSource;i<NewEndSource;i+=PAGE_SIZE)
		{
			sourceframe=get_frame_info(page_directory,(NewStartSource),&sourceptr);
			get_page_table(page_directory,(NewStartDest),&pagetabledest);
			if(pagetabledest==NULL)
			{
				create_page_table(page_directory,NewStartDest);
			}

			map_frame(page_directory,sourceframe,(NewStartDest),perms);
			NewStartSource+=PAGE_SIZE;
			NewStartDest+=PAGE_SIZE;

		}
		return 0;







}

//===============================
// 4) ALLOCATE CHUNK IN RAM:
//===============================
//This function should allocate in RAM the given range [va, va+size)
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int allocate_chunk(uint32* page_directory, uint32 va, uint32 size, uint32 perms)
{

	//panic("allocate_chunk() is not implemented yet...!!");
	struct FrameInfo* destframe;
	struct FrameInfo* NewFrame;
	    uint32* destptr;
	    uint32* PageTablePtr;
	    ///////////////////////
	    uint32 NewVaDown=ROUNDDOWN(va,PAGE_SIZE);
	    uint32 NewVaUp=ROUNDUP(va+size,PAGE_SIZE);



		for(int i=NewVaDown;i<NewVaUp;i+=PAGE_SIZE)
		{
			destframe=get_frame_info(page_directory,i,&destptr);
			if(destframe!=0)
			{
				return -1;

			}



		}
		NewVaDown=ROUNDDOWN(va,PAGE_SIZE);
	    NewVaUp=ROUNDUP(va+size,PAGE_SIZE);

		for(int i=NewVaDown;i<NewVaUp;i+=PAGE_SIZE)
		{

			get_page_table(page_directory,i,&PageTablePtr);
			if(PageTablePtr==NULL)
			{
				create_page_table(page_directory,i);
			}
			allocate_frame(&NewFrame);
			//newframe var dah 3alshan 5atr function el vitrual elly fe kheap mesh aktr
			NewFrame->va=i;
			map_frame(page_directory,NewFrame,i,perms);

		}


return 0;


}








/*BONUS*/
//=====================================
// 5) CALCULATE ALLOCATED SPACE IN RAM:
//=====================================
void calculate_allocated_space(uint32* page_directory, uint32 sva, uint32 eva, uint32 *num_tables, uint32 *num_pages)
{
	//TODO: [PROJECT MS2 - BONUS] [CHUNK OPERATIONS] calculate_allocated_space
	// Write your code here, remove the panic and write your code
	//panic("calculate_allocated_space() is not implemented yet...!!");
	uint32 mysva=ROUNDDOWN(sva,PAGE_SIZE);
	uint32 myeva=ROUNDUP(eva,PAGE_SIZE);
	uint32 numberframes=0;
	uint32 numbertables=0;
	struct FrameInfo* frame;
	uint32 *ptrpagetable;

	for(uint32 i=mysva;i<myeva;i+=PAGE_SIZE)

	{

		frame=get_frame_info(page_directory,i,&ptrpagetable);
		if(frame!=NULL)
		{
			numberframes++;


		}

	}
	 mysva=ROUNDDOWN(sva,PAGE_SIZE*1024);
	 myeva=ROUNDUP(eva,PAGE_SIZE*1024);
	for(uint32 i=mysva;i<myeva;i+=(PAGE_SIZE*1024))

		{

			get_page_table(page_directory,i,&ptrpagetable);
			if(ptrpagetable!=NULL)
			{
				numbertables++;


			}

		}
	*num_tables=numbertables;
	*num_pages=numberframes;















}

/*BONUS*/
//=====================================
// 6) CALCULATE REQUIRED FRAMES IN RAM:
//=====================================
// calculate_required_frames:
// calculates the new allocation size required for given address+size,
// we are not interested in knowing if pages or tables actually exist in memory or the page file,
// we are interested in knowing whether they are allocated or not.
uint32 calculate_required_frames(uint32* page_directory, uint32 sva, uint32 size)
{
	//TODO: [PROJECT MS2 - BONUS] [CHUNK OPERATIONS] calculate_required_frames
	// Write your code here, remove the panic and write your code
	//panic("calculate_required_frames() is not implemented yet...!!");

	uint32 mysva=ROUNDDOWN(sva,PAGE_SIZE);
	uint32 myeva=ROUNDUP(size+sva,PAGE_SIZE);
	uint32 numberframes=0;
	uint32 numbertables=0;
	struct FrameInfo* frame;
	uint32 *ptrpagetable;

	for(uint32 i=mysva;i<myeva;i+=PAGE_SIZE)

	{

		frame=get_frame_info(page_directory,i,&ptrpagetable);
		if(frame==NULL)
		{
			numberframes++;


		}

	}
	 mysva=ROUNDDOWN(sva,PAGE_SIZE*1024);
	 myeva=ROUNDUP(size+sva,PAGE_SIZE*1024);
	for(uint32 i=mysva;i<myeva;i+=(PAGE_SIZE*1024))

		{

			get_page_table(page_directory,i,&ptrpagetable);
			if(ptrpagetable==NULL)
			{
				numberframes++;

			}

		}
	return numberframes;

























}

//=================================================================================//
//===========================END RAM CHUNKS MANIPULATION ==========================//
//=================================================================================//

/*******************************/
/*[2] USER CHUNKS MANIPULATION */
/*******************************/

//======================================================
/// functions used for USER HEAP (malloc, free, ...)
//======================================================

//=====================================
// 1) ALLOCATE USER MEMORY:
//=====================================
void allocate_user_mem(struct Env* e, uint32 virtual_address, uint32 size)
{
	// Write your code here, remove the panic and write your code
	panic("allocate_user_mem() is not implemented yet...!!");
}

//=====================================
// 2) FREE USER MEMORY:
//=====================================
void free_user_mem(struct Env* e, uint32 virtual_address, uint32 size)
{
	//TODO: [PROJECT MS3] [USER HEAP - KERNEL SIDE] free_user_mem
	// Write your code here, remove the panic and write your code
	//panic("free_user_mem() is not implemented yet...!!");

	//This function should:
	//1. Free ALL pages of the given range from the Page File
	for(int i =0;i<size;i+=PAGE_SIZE)
	{
		pf_remove_env_page(e, virtual_address+i);
	}

	//2. Free ONLY pages that are resident in the working set from the memory

	for(int i =0;i<size;i+=PAGE_SIZE)
		{

		env_page_ws_invalidate(e,virtual_address+i);
		unmap_frame(e->env_page_directory,(virtual_address+i));

		}
	//3. Removes ONLY the empty page tables (i.e. not used) (no pages are mapped in the table)
	 int y=0;
	 uint32* ptr1;
	for(int i =0;i<size;i+=PAGE_SIZE*1024)
	{
	    ptr1=NULL;
	    get_page_table(e->env_page_directory,(virtual_address+i),&ptr1);
		y=0;
		if(ptr1!=NULL)
		{
			//hna batcheck how fady wala la2
			for(int x=0;x<1024;x++)
			{
				if(ptr1[x]!=0)
				{
					y=1;
					break;
				}

			}
			if(y==0)
			{
				e->env_page_directory[PDX(virtual_address+i)]=0;

				kfree((void*)(ptr1));



			}

		}

	}







}

//=====================================
// 2) FREE USER MEMORY (BUFFERING):
//=====================================
void __free_user_mem_with_buffering(struct Env* e, uint32 virtual_address, uint32 size)
{
	// your code is here, remove the panic and write your code
	panic("__free_user_mem_with_buffering() is not implemented yet...!!");

	//This function should:
	//1. Free ALL pages of the given range from the Page File
	//2. Free ONLY pages that are resident in the working set from the memory
	//3. Free any BUFFERED pages in the given range
	//4. Removes ONLY the empty page tables (i.e. not used) (no pages are mapped in the table)
}

//=====================================
// 3) MOVE USER MEMORY:
//=====================================
void move_user_mem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
{
	//TODO: [PROJECT MS3 - BONUS] [USER HEAP - KERNEL SIDE] move_user_mem
	//your code is here, remove the panic and write your code
	panic("move_user_mem() is not implemented yet...!!");

	// This function should move all pages from "src_virtual_address" to "dst_virtual_address"
	// with the given size
	// After finished, the src_virtual_address must no longer be accessed/exist in either page file
	// or main memory

	/**/
}

//=================================================================================//
//========================== END USER CHUNKS MANIPULATION =========================//
//=================================================================================//

