#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


#define MAX_ALLOCATOR_MEMORY (64 * 1024)

typedef struct mem_header_
{
    uint16_t	    size;
    bool	    is_free;
    /* Returned pointer should be multiple of 32 or 32-byte alligned.
     * num_padding : Indicates the number of bytes padded to make it alligned
     */
    uint16_t	    num_padding;
    struct	    mem_header *next;
    void	    *start;
}mem_header_t;

mem_header_t	*allocator;

void *
init_allocator(mem_header_t *prev)
{
    mem_header_t    *head;
    void	    *block;
    int		    rem;

    block = malloc(sizeof(MAX_ALLOCATOR_MEMORY));
    if(block == NULL)
    {
	fprintf(stderr, "Allocation failed!!!\n");
	exit(-1);
    }
    head = block;
    head->is_free = true;
    rem = (unsigned int)head % 32;
    head->num_padding = 32 - rem;
    head->size = MAX_ALLOCATOR_MEMORY - head->num_padding - sizeof(mem_header_t);

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
    int		    rem;
    void	    *tmp=NULL;
    mem_header_t    *head, *new_block;
    void	    *ret_ptr = NULL;
    
    head = (mem_header_t *)allocator;
    while(head != NULL)
    {
	if(head->is_free == true)
	{
	    if(head->size > size)
	    {
		/* We have more than we need. Divide it into two blocks */
		int extra_avail_mem = head->size - size;
		tmp = head;
		ret_ptr = tmp + sizeof(mem_header_t) + head->num_padding;
		head->next = tmp + sizeof(mem_header_t) + head->num_padding + size;
		head = head->next;
		
		rem = (unsigned int)head % 32;
		head->size = extra_avail_mem;
		head->num_padding = 32 - rem;
		/* Here size will be negative if this block does not have sufficint space 
		 * to hold header and padding bytes
		 */
		extra_avail_mem = extra_avail_mem - head->num_padding - sizeof(mem_header_t);
		if(extra_avail_mem < 0)
		    head->size = 0;
		
		return ret_ptr;
	    }
	    else if(head->size == size)
	    {
		tmp = head;
		ret_ptr = tmp + sizeof(mem_header_t) + head->num_padding;
		return ret_ptr;
	    }
	}
	    
	head = head->next;
    }

    new_block = init_allocator(NULL);
    new_block =  new_block + sizeof(mem_header_t) + new_block->num_padding;

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
	printf("Allocated pointer   :   %x\n", ptr);
	if((unsigned int)ptr%32 == 0)
	    printf("32-byte Alligned\n");
	else
	    printf("***error [%x] is not 32-byte alligned\n", ptr);
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
    printf("Allocated pointer	:   %x\n", ptr);

    test_new_malloc();
    return 0;
}

