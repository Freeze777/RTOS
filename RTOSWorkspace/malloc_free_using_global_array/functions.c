
/** @file functions.c
 *  @brief Function definitions for dynamic memory management.
 *
 *  This contains the function definitons for the dynamic memory 
 *  allocations and deallocations including utility functions.
 *
 *  @author Freeze Francis
 * 
 */



#include "functions.h"

/** @brief The buffer array used for memory allocation.
 * 
 */
static char buffer[BUFF_SIZE]={0};

/** @brief Points to the base address of the array
 * 
 */
static  metadata * heap_base=(metadata *)buffer;

/** @brief Keeps tracks of the tail pointer of the linked list of allocated block
 * 
 */
static metadata * last=NULL;
/** 
 *  @brief The function which searches in the list of memory blocks which fits the requested size.
 *  Used during a malloc() or calloc() calls to get the first fit.
 *  @param size The requested size
 *  @return pointer to the free block which fits the size
 */
static metadata * search_freespace(size_t size);

/** 
 *  @brief The function which fuses two adjacent free blocks into single free block.
 *  Used during a call to free() 
 *  @param ptr pointer to the first free block
 *  @return void
 */
static void fuse(metadata * ptr);
/** 
 *  @brief The function which splits a large memory block into two smaller block based on the threshold value.
 *  Used during a call to malloc() and calloc(). 
 *  @param ptr pointer to the block to be split
 *  @return void
 */

static void split(metadata * ptr,size_t size);

/** 
 *  @brief The function which checks the validity of the block starting address.
 *  Used during a call to free() and thus avoids memory corruption.
 *  @param to_free pointer to the block to be freed
 *  @return validity (0-invalid 1-valid)
 */
static int verifyBlockAddress(metadata * to_free,metadata ** prev);


/** 
 *  @brief The variable keep track of freespace in the array
 *  
  */
static size_t freespace=BUFF_SIZE;

static int first=1;




void * my_malloc(size_t size)
{
    printf("\nmalloc for %lu\n",size);
    if(size==0)
        return NULL;

    metadata * block=NULL;


    if(!first){
        block=search_freespace(size);
        if(block==NULL)
        {
            block=(metadata *)(last->size+(char*)(last+1));
            last->next=block;
            last=block;
            block->next=NULL;
            block->size=size;
            freespace-=(size+sizeof(metadata));
        }else
        {
            if(block->size >= size+THRESHOLD)
            {   freespace-=block->size;
                split(block,size);
                freespace+=block->next->size;
            }else{
                freespace-=size;
            }
        }

    }else{

        first=0;
        block=heap_base;
        block->size=size;
        block->next=NULL;
        last=block;
        freespace-=(size+sizeof(metadata));


    }


    
    block->free=0;


    return block+1;

}
void * my_calloc(size_t n,size_t size){
    printf("\ncalloc for %lu\n",n*size);
    size_t total=n*size;
    char * temp=(char *)my_malloc(total);

    for (int var = 0; var < total ; ++var)
        temp[var]=0;

    return (void*)temp;
}

void my_free(void *ptr)
{

    if(ptr==NULL)
        return;

    metadata * to_free=(metadata *)ptr - 1;
    metadata * prev=NULL;

    if(to_free>=heap_base && to_free <=last){
        if(verifyBlockAddress(to_free ,&prev))
        {
        printf("\nfree for %lu \n",to_free->size);

        to_free->free=1;
        freespace+=to_free->size;
        fuse(to_free);

        if((prev!=NULL)&&prev->free==1)
        fuse(prev);
        }
        else
        {
           printf("\nInvalid address.. !!\n");
        }
    }else
    {
        printf("\nInvalid address.. !!\n");
    }


    deframent_my_heap();
}

static int verifyBlockAddress(metadata * to_free,metadata ** prev){
    metadata * temp=heap_base;

    while(temp!=NULL)
    {
        if(to_free==temp)
            return 1;

        *prev=temp;
        temp=temp->next;
    }
    return 0;


}


void * my_realloc(void *ptr,size_t size){


    if(ptr==NULL)
        return my_malloc(size);

    if(size==0)
    {
        my_free(ptr);
        return NULL;
    }

    metadata * block=(metadata *)ptr -1;
    printf("\nrealloc from blocksize: %lu  to size: %lu\n",block->size,size);

    if(block->size < size)//expand
    {
        if(block->next->free==1 && block->next->size >= ( size - block->size -sizeof(metadata)))
        {
            block->size+=sizeof(metadata)+block->next->size;
            freespace-=block->next->size;

            if(block->next->next==NULL)
                last=block;

            block->next=block->next->next;
            //print_memory_contents();

            if(block->size >= size+THRESHOLD)
            {    split(block,size);
                freespace+=block->next->size;
            }
            return block+1;

        }
        char * chunk=(char *)my_malloc(size);
        memcpy(chunk,block+1,block->size);
        my_free(block+1);

        return chunk;

    }else if (block->size > size)//shrink
    {
        if(block->size >= size+THRESHOLD)
        {
            split(block,size);
            freespace+=block->next->size;
            return block+1;
        }

        char * chunk=(char *)my_malloc(size);
        memcpy(chunk,block+1,block->size);
        my_free(block+1);

        return chunk;

    }

}


static metadata * search_freespace(size_t size){
    metadata * temp=heap_base;
    while((temp!=NULL)&&!(temp->free && temp->size >= size))
    {
        temp=temp->next;

    }
    return temp;
}
static void fuse(metadata * ptr){


    while((ptr->next!=NULL) && ptr->next->free==1)
    {
        printf("\nfusing %lu & %lu\n",ptr->size,ptr->next->size);
        ptr->size+=ptr->next->size+sizeof(metadata);
        ptr->next=ptr->next->next;
        freespace+=sizeof(metadata);
    }
    if(ptr->next==NULL)
    {
        last=ptr;
    }

}
static void split(metadata *ptr,size_t size)
{
    char * temp=(char *) ptr;
    temp+=(sizeof(metadata)+size);
    metadata * chunk=(metadata *) temp;

    chunk->next=ptr->next;
    ptr->next=chunk;

    chunk->free=1;
    chunk->size=(ptr->size)-(size+sizeof(metadata));

    ptr->size=size;

    if(chunk->next==NULL)
    {
        last=chunk;
    }

}

void deframent_my_heap(void){

    printf("\ndefragmenting memory\n");
    metadata * temp=heap_base;
    while(temp!=NULL)
    {
        if(temp->next!=NULL && temp->free && temp->next->free)
         fuse(temp);
        temp=temp->next;
    }
}

size_t free_space_in_my_heap(void){

    return freespace;

}

void print_memory_contents(void){
    printf("\n");
    metadata * temp=heap_base;
    while(temp!=NULL)
    {
        printf("%lu %d\n",temp->size,temp->free);
        temp=temp->next;
    }
printf("\nfree space:%lu B\n",free_space_in_my_heap());
}
