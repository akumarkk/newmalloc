#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define DEBUG 1

#define MAX_ALLOCATOR_MEMORY (64 * 1024)

#define debug(fmt,...)\
    do{\
	if(DEBUG)\
	    fprintf(stdout, "%s(%d) : " fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__); \
    }while(0)

typedef struct mem_header_
{
    uint16_t	    size;
    bool	    is_free;
    /* Returned pointer should be multiple of 32 or 32-byte alligned.
     * num_padding : Indicates the number of bytes padded to make it alligned
     */
    uint16_t	    num_padding;
    struct	    mem_header_ *next;
    void	    *start;
}memory_header_t;

void *allocator;

void *
init_allocator(memory_header_t *prev, uint16_t size)
{
    memory_header_t     *head, *tmp;
    void		*block;
    int			rem;
    int			available_mem = 0;		

    block = malloc(sizeof(MAX_ALLOCATOR_MEMORY));
    if(block == NULL)
    {
	fprintf(stderr, "Allocation failed!!!\n");
	exit(-1);
    }

    debug("Requested size   =  %u", size);
    debug("Allocated block  =  %p", block);
    debug("Sizeof header    =  %lu", sizeof(memory_header_t));

    head = block;
    head->is_free = false;

    rem = (size_t)head % 32;
    if((size_t)head % 32)
	head->num_padding = 32 - rem; 
    
    available_mem = MAX_ALLOCATOR_MEMORY - head->num_padding - sizeof(memory_header_t);

    debug("Number of bytes paddding	=   %d", head->num_padding);
    debug("Size allocated		=   %d", head->size);

    if(available_mem > size)
    {
	/* Lets break it into two pieces */
	tmp = (memory_header_t *)((char *)head + head->num_padding + sizeof(memory_header_t) + size);
	debug("Next block		=   %p", tmp);

	head->next = tmp;
	tmp->is_free = true;

	if((size_t)tmp % 32)
	    tmp->num_padding = 32 - ((size_t)tmp % 32);
	else
	    tmp->num_padding = 0;
	tmp->next = NULL;
	tmp->size = available_mem - sizeof(memory_header_t) - tmp->num_padding;
    }
    else
	head->next = NULL;
	

    if(prev == NULL)
	allocator = block;
    else
	/* This is not the first time. This is for additional space */
	prev->next = block;
    return block;
}

void *
allocate_mem(size_t size)
{
    void	    *new_block=NULL;
    int		    rem;
    void	    *tmp=NULL;
    memory_header_t    *head;
    void	    *ret_ptr = NULL;
    int		    num_padding=0;
    
    head = (memory_header_t *)allocator;
    while(head != NULL)
    {
	debug("Looking for free memory block [%p]", head);
	if(head->is_free == true)
	{
	    if(head->size > size)
	    {
		/* We have more than we need. Divide it into two blocks */
		int extra_avail_mem = head->size - size;
		tmp = head;
		ret_ptr = tmp + sizeof(memory_header_t) + head->num_padding;
		head->next = tmp + sizeof(memory_header_t) + head->num_padding + size;
		head = head->next;
		
		if((size_t)head % 32)
		    head->num_padding = 32 - ((size_t)head % 32);
		else
		    head->num_padding =0;
		
		/* Here size will be negative if this block does not have sufficint space 
		 * to hold header and padding bytes
		 */
		extra_avail_mem = extra_avail_mem - head->num_padding - sizeof(memory_header_t);
		if(extra_avail_mem < 0)
		    head->size = 0;
		else
		    head->size = extra_avail_mem;
		head->next = NULL;
		
		return ret_ptr;
	    }
	    else if(head->size == size)
	    {
		tmp = head;
		ret_ptr = tmp + sizeof(memory_header_t) + head->num_padding;
		return ret_ptr;
	    }
	}
	    
	head = head->next;
    }

    new_block = init_allocator(NULL, size);
    debug("Allocated pointer		    =	%p", new_block);
    num_padding = ((memory_header_t *)new_block)->num_padding;

    new_block =  (char *)new_block + sizeof(memory_header_t) + num_padding;
    debug("Newly allocated return pointer   =	%p\n", new_block);

    return new_block;
}


void *
new_malloc(size_t size)
{
    void *ptr = NULL;
    if(size <= 0)
	return NULL;

    ptr = allocate_mem(size);
    return ptr;
}

void
test_new_malloc()
{
    void    *ptr = NULL;
    int     size;

    while(1)
    {
	printf("Size of memory to allocate  : ");
	scanf("%d", &size);
	ptr = new_malloc(size);
	printf("Allocated pointer   :   %p\n", ptr);
	if((unsigned int)ptr%32 == 0)
	    printf("32-byte Alligned\n");
	else
	    printf("***error [%p] is not 32-byte alligned\n", ptr);
    }
}

int
main()
{
    void    *ptr = NULL;
    int	    size;

    printf("Size of memory to allocate	: ");
    scanf("%d", &size);

    ptr = new_malloc(size);
    printf("Allocated pointer	:   %p\n", ptr);

    test_new_malloc();
    return 0;
}

