#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

//==================================================================//
//==================================================================//
//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)//
//==================================================================//
//==================================================================//

void initialize_dyn_block_system()
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] initialize_dyn_block_system
	// your code is here, remove the panic and write your code
	//kpanic_into_prompt("initialize_dyn_block_system() is not implemented yet...!!");

	//[1] Initialize two lists (AllocMemBlocksList & FreeMemBlocksList) [Hint: use LIST_INIT()]

	LIST_INIT(&AllocMemBlocksList);
	LIST_INIT(&FreeMemBlocksList);

#if STATIC_MEMBLOCK_ALLOC
	//DO NOTHING
#else
	/*[2] Dynamically allocate the array of MemBlockNodes
	 * 	remember to:
	 * 		1. set MAX_MEM_BLOCK_CNT with the chosen size of the array
	 * 		2. allocation should be aligned on PAGE boundary
	 * 	HINT: can use alloc_chunk(...) function
	 */
	MAX_MEM_BLOCK_CNT=NUM_OF_KHEAP_PAGES;
	MemBlockNodes=(struct MemBlock*)KERNEL_HEAP_START;
	int arrsize=sizeof(struct MemBlock)*NUM_OF_KHEAP_PAGES;
	arrsize=ROUNDUP(arrsize,PAGE_SIZE);



	allocate_chunk(ptr_page_directory,KERNEL_HEAP_START,arrsize,PERM_WRITEABLE|PERM_PRESENT);


#endif
	//[3] Initialize AvailableMemBlocksList by filling it with the MemBlockNodes
	//[4] Insert a new MemBlock with the remaining heap size into the FreeMemBlocksList
	initialize_MemBlocksList(NUM_OF_KHEAP_PAGES);
	struct MemBlock* i;

	i=AvailableMemBlocksList.lh_last;

	LIST_REMOVE(&AvailableMemBlocksList,i);

	i->size=(NUM_OF_KHEAP_PAGES*4*1024)-arrsize;


     i->sva=KERNEL_HEAP_START+arrsize;

	LIST_INSERT_TAIL(&FreeMemBlocksList,i);







}

void* kmalloc(unsigned int size)
{
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");
	//change this "return" according to your answer

    if(isKHeapPlacementStrategyFIRSTFIT())
    {
    	uint32 newsize=ROUNDUP(size,PAGE_SIZE);


    	struct MemBlock* block=alloc_block_FF(newsize);



    	if(block!=NULL)
    	{
    	    insert_sorted_allocList(block);
    		allocate_chunk(ptr_page_directory,block->sva,size,PERM_WRITEABLE|PERM_PRESENT);

    		return (void*)block->sva;

    	}
    	else
    		return NULL;







    }
    else if (isKHeapPlacementStrategyBESTFIT())
    {
    	uint32 newsize=ROUNDUP(size,PAGE_SIZE);


    	  struct MemBlock* block=alloc_block_BF(newsize);



    	    	if(block!=NULL)
    	    	{
    	    		insert_sorted_allocList(block);
    	    		allocate_chunk(ptr_page_directory,block->sva,size,PERM_WRITEABLE|PERM_PRESENT);
    	    		return (void*)block->sva;

    	    	}
    	    	else
    	     		return NULL;







    }
    else if (isKHeapPlacementStrategyNEXTFIT())
        {
        	uint32 newsize=ROUNDUP(size,PAGE_SIZE);


        	struct MemBlock* block=alloc_block_NF(newsize);



        	if(block!=NULL)
        	{
        	    insert_sorted_allocList(block);
        		allocate_chunk(ptr_page_directory,block->sva,size,PERM_WRITEABLE|PERM_PRESENT);
        		return (void*)block->sva;

        	}
        	else
        		return NULL;







        }


    return NULL;





}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] kfree
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");


		struct MemBlock* ptr=NULL;

		ptr=find_block(&AllocMemBlocksList,(uint32)virtual_address);
		if(ptr!=NULL)
		{
			//LIST_REMOVE(&AllocMemBlocksList,ptr);

				for(int i=0;i<ptr->size;i+=PAGE_SIZE)
				{
					 unmap_frame(ptr_page_directory,(uint32)virtual_address+i);

				}
				  LIST_REMOVE(&AllocMemBlocksList,ptr);

			    insert_sorted_with_merge_freeList(ptr);



		}








}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] kheap_virtual_address
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");

	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details
	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================

struct FrameInfo* x;

x=to_frame_info(physical_address);

return x->va;








}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] kheap_physical_address
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details

	uint32 x=virtual_to_physical(ptr_page_directory,virtual_address);
	return x;

}


void kfreeall()
{
	panic("Not implemented!");

}

void kshrink(uint32 newSize)
{
	panic("Not implemented!");
}

void kexpand(uint32 newSize)
{
	panic("Not implemented!");
}




//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{

	//TODO: [PROJECT MS2 - BONUS] [KERNEL HEAP] krealloc
	// Write your code here, remove the panic and write your code
	//panic("krealloc() is not implemented yet...!!");
	if(virtual_address==NULL)
	{

		return kmalloc(new_size);


	}

	if(new_size==0)
	{

		kfree(virtual_address);
		return NULL;
	}


	struct MemBlock* x=find_block(&AllocMemBlocksList,(uint32)virtual_address);
	if(x->size==new_size)
	{
		return virtual_address;

	}
	//law el size el 2adem akbr mn el size el gdeid
	if(x->size>new_size)
	{

		uint32 outsize=x->sva+new_size;


		kfree((void*)outsize);


		return virtual_address;

	}

   uint32 nextspace=x->sva+x->size;
	struct MemBlock* nextblock=find_block(&FreeMemBlocksList,nextspace);
	uint32 extrasize=(new_size)-(x->size);
	uint32 xsize=x->size;
	if(x->size<new_size)
	{
		if(nextblock!=NULL&&nextblock->size>=(new_size)-(x->size))
			{



			LIST_REMOVE(&FreeMemBlocksList,nextblock);

			insert_sorted_allocList(nextblock);

		   allocate_chunk(ptr_page_directory,nextblock->sva,(new_size)-(x->size),PERM_WRITEABLE|PERM_PRESENT);


					return  virtual_address;
			}




				if(nextblock==NULL||nextblock->size<(new_size)-(x->size))
				{
					struct MemBlock* y=alloc_block_BF(new_size);
					if(y==NULL)
						return NULL;
				    insert_sorted_allocList(y);
					cut_paste_pages(ptr_page_directory,x->sva,y->sva,x->size/PAGE_SIZE);
					LIST_REMOVE(&AllocMemBlocksList,x);
					insert_sorted_with_merge_freeList(x);
		 allocate_chunk(ptr_page_directory,y->sva+xsize,extrasize,PERM_WRITEABLE|PERM_PRESENT);
		 cprintf("ba3d el chunk \n ");


		  return (void*)y->sva;



				}



	}






			return NULL;
}















