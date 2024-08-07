#include <inc/lib.h>
/////

uint32 globalarr[10]={0,0,0,0,0,0,0,0,0,0};



/////


//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

int FirstTimeFlag = 1;
void InitializeUHeap()
{
	if(FirstTimeFlag)
	{
		initialize_dyn_block_system();
		cprintf("DYNAMIC BLOCK SYSTEM IS INITIALIZED\n");
#if UHP_USE_BUDDY
		initialize_buddy();
		cprintf("BUDDY SYSTEM IS INITIALIZED\n");
#endif
		FirstTimeFlag = 0;
	}
}

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//=================================
void initialize_dyn_block_system()
{
	//TODO: [PROJECT MS3] [USER HEAP - USER SIDE] initialize_dyn_block_system
	// your code is here, remove the panic and write your code
	//panic("initialize_dyn_block_system() is not implemented yet...!!")

	//[1] Initialize two lists (AllocMemBlocksList & FreeMemBlocksList) [Hint: use LIST_INIT()]
	LIST_INIT(&AllocMemBlocksList);
	LIST_INIT(&FreeMemBlocksList);
	//[2] Dynamically allocate the array of MemBlockNodes at VA USER_DYN_BLKS_ARRAY
	//	  (remember to set MAX_MEM_BLOCK_CNT with the chosen size of the array)
	MAX_MEM_BLOCK_CNT=NUM_OF_UHEAP_PAGES;
	MemBlockNodes=(struct MemBlock*)USER_DYN_BLKS_ARRAY;
	int arrsize=sizeof(struct MemBlock)*NUM_OF_UHEAP_PAGES;
	arrsize=ROUNDUP(arrsize,PAGE_SIZE);
	sys_allocate_chunk(USER_DYN_BLKS_ARRAY,arrsize,PERM_USER|PERM_WRITEABLE);

	//[3] Initialize AvailableMemBlocksList by filling it with the MemBlockNodes

	initialize_MemBlocksList(NUM_OF_UHEAP_PAGES);
		struct MemBlock* i;

		i=AvailableMemBlocksList.lh_last;

		LIST_REMOVE(&AvailableMemBlocksList,i);

		i->size=(NUM_OF_UHEAP_PAGES*4*1024);


	     i->sva=USER_HEAP_START;

	     //[4] Insert a new MemBlock with the heap size into the FreeMemBlocksList

		LIST_INSERT_TAIL(&FreeMemBlocksList,i);


}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================

void* malloc(uint32 size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================
	//==============================================================

	//TODO: [PROJECT MS3] [USER HEAP - USER SIDE] malloc
	// your code is here, remove the panic and write your code
	//if(isKHeapPlacementStrategyFIRSTFIT())

	 if(sys_isUHeapPlacementStrategyFIRSTFIT())
	    {
	    	uint32 newsize=ROUNDUP(size,PAGE_SIZE);


	    	struct MemBlock* block=alloc_block_FF(newsize);



	    	if(block!=NULL)
	    	{
	    	    insert_sorted_allocList(block);
	    		return (void*)block->sva;

	    	}
	    	else
	    		return NULL;




	    }
	 return NULL;

}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	FROM main memory AND free pages from page file then switch back to the user again.
//
//	We can use sys_free_user_mem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls free_user_mem() in
//		"kern/mem/chunk_operations.c", then switch back to the user mode here
//	the free_user_mem function is empty, make sure to implement it.
void free(void* virtual_address)
{
	//TODO: [PROJECT MS3] [USER HEAP - USER SIDE] free
	// your code is here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");
	struct MemBlock* ptr=NULL;

			ptr=find_block(&AllocMemBlocksList,(uint32)virtual_address);
			if(ptr!=NULL)
			{
						sys_free_user_mem(ptr->sva,ptr->size);

					  LIST_REMOVE(&AllocMemBlocksList,ptr);

				    insert_sorted_with_merge_freeList(ptr);



			}

}


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================

	//TODO: [PROJECT MS3] [SHARING - USER SIDE] smalloc()
	// Write your code here, remove the panic and write your code
	//panic("smalloc() is not implemented yet...!!");
	if(sys_isUHeapPlacementStrategyFIRSTFIT())
		    {
		    	uint32 newsize=ROUNDUP(size,PAGE_SIZE);


		    	struct MemBlock* block=alloc_block_FF(newsize);

		    	int check;



		    	if(block!=NULL)
		    	{
		    		check=sys_createSharedObject(sharedVarName,newsize,isWritable,(void*)block->sva);
		    		if(check==1000)
		    		{
		    			return NULL;
		    		}
		    	    insert_sorted_allocList(block);
		    	    ///////////////////
//
//		    	    for(int i=0;i<10;i++)
//		    	    {
//		    	    	if(globalarr[i]==0)
//		    	    	{
//		    	    		globalarr[i]=block->sva;
//		    	    		break;
//		    	    	}
//		    	    }

		    	    globalarr[check]=block->sva;


		    	    ///////////////////
		    		return (void*)block->sva;

		    	}
		    	else
		    		return NULL;




		    }
		 return NULL;




}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================

	//TODO: [PROJECT MS3] [SHARING - USER SIDE] sget()
	// Write your code here, remove the panic and write your code
	//panic("sget() is not implemented yet...!!");
	int thesize;
	thesize=sys_getSizeOfSharedObject(ownerEnvID,sharedVarName);

	if(thesize!=0)
	{
		if(sys_isUHeapPlacementStrategyFIRSTFIT())
				    {
				    	uint32 newsize=ROUNDUP(thesize,PAGE_SIZE);


				    	struct MemBlock* block=alloc_block_FF(newsize);


				    	if(block!=NULL)
				    	{
				    	    insert_sorted_allocList(block);

				    	    uint32 check;
				    	  check=sys_getSharedObject(ownerEnvID,sharedVarName,(void*)block->sva);
				    	  if(check!=1000)
				    	  {
				    		  ////////////////////


								globalarr[check]=block->sva;



				    		  ////////////////////

				    			return (void*)block->sva;
				    	  }
				    	  else
				    		  return NULL;



				    	}
				    	else
				    		return NULL;




				    }
		else
			return NULL;

	}

	else
		return NULL;




}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//


//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================
	// [USER HEAP - USER SIDE] realloc
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
}


//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//TODO: [PROJECT MS3 - BONUS] [SHARING - USER SIDE] sfree()

	// Write your code here, remove the panic and write your code
	//panic("sfree() is not implemented yet...!!");
	struct MemBlock* ptr=NULL;

				ptr=find_block(&AllocMemBlocksList,(uint32)virtual_address);
				if(ptr!=NULL)
				{
					int MyIndex;
					//hna bageib el ID bta3 el object b el va elly ma3ya da
					///////////////////////////
					for(int i=0;i<10;i++)
					{

						if(globalarr[i]==ptr->sva)
						{
							MyIndex=i;
							break;
						}


					}
					////////////////////////////


				    	int y=sys_freeSharedObject(MyIndex,(void*)ptr->sva);
				    	if((y==1)||(y==0))
				    	{
				    		 LIST_REMOVE(&AllocMemBlocksList,ptr);

				    		insert_sorted_with_merge_freeList(ptr);

				    	}



						    /////
					     	if((y==1)||(y==E_SHARED_MEM_NOT_EXISTS))
					     	{
					     		  globalarr[MyIndex]=0;

					     	}







				}

}




//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//
void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");
}
