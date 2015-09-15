#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>

typedef struct mem_header_
{
    uint16_t	    size;
    bool	    is_free;
    uint16_t	    num_padding;
    struct mem_header *next;
}mem_header_t;

mem_header_t	*allocator;

void
init_allocator()
{
    allocator = malloc(sizeof(MAX_ALLOCATOR_MEMORY));
    if(allocator == NULL)
    {
	fprintf(stderr, "Allocation failed!!!\n");
	exit(-1);
    }
}


void *
malloc(size_t size)
{
    if(size <= 0)
	return NULL;

    if(allocator == NULL)
    {
	init_allocator();
    }

}

