#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include <sys/mman.h>
#include <execinfo.h>
#include <gc/gc.h>
#include <gc/gc_typed.h>
#include "citron.h"

GC_descr VT_descr, PT_descr;
int gc_types_initialized = 0;
/**
 * Heap Object, represents dynamic memory.
 */

struct memBlock {
	size_t size;
	void *space;
};


typedef struct memBlock memBlock;

memBlock *memBlocks = NULL;
size_t numberOfMemBlocks = 0;
size_t maxNumberOfMemBlocks = 0;

void sttrace_print(void *ptr)
{
	if (likely(ptr))
		return;
	printf("MEMORY ALLOC %p\n", ptr);
	void *array[99999];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);

	for (i = 0; i < size; i++)
		printf("%s\n", strings[i]);

	free(strings);
}

/**
 * Heap allocate raw memory
 * Allocates a slice of memory having the specified size in bytes.
 * The memory will be zeroed (calloc is used).
 *
 * If the specified number of bytes cannot be allocated, the program
 * will end with exit 1.
 *
 * If the specified number of bytes causes the total number of allocated
 * bytes to exceed the GC thresold, the garbage collector will attempt to free
 * memory.
 *
 * If the specified number of bytes causes the total number of allocated
 * bytes to exceed the predetermined memory limit, the program will exit with
 * return code 1.
 *
 * This function will track the allocated bytes to monitor memory
 * management.
 *
 * @param uintptr_t size memory size
 *
 * @return void*
 */
void *ctr_heap_allocate(size_t size)
{
	if (!gc_types_initialized) setup_types();
	return GC_MALLOC(size);
}

void *ctr_heap_allocate_typed(size_t size, int otype) {
	switch(otype) {
		case CTR_OBJECT_TYPE_OTNIL:
		case CTR_OBJECT_TYPE_OTBOOL:
		case CTR_OBJECT_TYPE_OTNUMBER:
		case CTR_OBJECT_TYPE_OTOBJECT:
			return GC_malloc_explicitly_typed(size, VT_descr);
		case CTR_OBJECT_TYPE_OTSTRING:
		case CTR_OBJECT_TYPE_OTNATFUNC:
		case CTR_OBJECT_TYPE_OTBLOCK:
		case CTR_OBJECT_TYPE_OTARRAY:
		case CTR_OBJECT_TYPE_OTEX:
		case CTR_OBJECT_TYPE_OTMISC:
			return GC_malloc_explicitly_typed(size, PT_descr);
		default: abort();
	}
}

void *create_shared_memory(size_t size)
{
	// Our memory buffer will be readable and writable:
	int protection = PROT_READ | PROT_WRITE | PROT_EXEC;
	int visibility = MAP_ANONYMOUS | MAP_SHARED;
	return mmap(NULL, size, protection, visibility, -1, 0);
}

void *ctr_heap_allocate_shared(size_t size)
{

	if (!gc_types_initialized) setup_types();
	volatile void *slice_of_memory;
	size_t *block_width;
	int q = sizeof(size_t);
	size += q;

	/* Check whether we can afford to allocate this much */
	ctr_gc_alloc = GC_get_heap_size() - GC_get_free_bytes() - GC_get_unmapped_bytes();

	if (ctr_gc_memlimit < ctr_gc_alloc) {
		if (CTR_LIMIT_MEM) {
			printf("Out of memory. Failed to allocate %lu bytes.\n",
			       size);
			ctr_print_stack_trace();
			abort();
			exit(1);
		} else {
			ctr_gc_memlimit *= 2;
		}
	}

	/* Perform allocation and check result */
	slice_of_memory = create_shared_memory(size);

	if (slice_of_memory == NULL) {
		printf
		    ("Out of memory. Failed to allocate %lu bytes (malloc failed). \n",
		     size);
		 abort();
		exit(1);
	}

	/* Store the width of the memory block in the slice itself so we can always find it */
	block_width = (size_t *) slice_of_memory;
	*(block_width) = size;
	/* Now move the new memory pointer behind the blockwidth */
	slice_of_memory = (void *)((char *)slice_of_memory + q);
	sttrace_print((void *)slice_of_memory);
	return (void *)slice_of_memory;
}

/**
 * Allocates memory on heap and tracks it for clean-up when
 * the program ends.
 */
void *ctr_heap_allocate_tracked(size_t size)
{
	void *space;
	space = ctr_heap_allocate(size);
	if (numberOfMemBlocks >= maxNumberOfMemBlocks) {
		if (memBlocks == NULL) {
			memBlocks = ctr_heap_allocate(sizeof(memBlock));
			maxNumberOfMemBlocks = 1;
		} else {
			maxNumberOfMemBlocks += 10;
			memBlocks =
			    ctr_heap_reallocate(memBlocks,
						(sizeof(memBlock) *
						 (maxNumberOfMemBlocks)));
		}
	}
	memBlocks[numberOfMemBlocks].space = space;
	memBlocks[numberOfMemBlocks].size = size;
	numberOfMemBlocks++;
	return space;
}

/**
 * Reallocates tracked memory on heap.
 * You need to provide a tracking ID.
 */
void *ctr_heap_reallocate_tracked(size_t tracking_id, size_t size)
{
	void *space;
	space = memBlocks[tracking_id].space;
	space = ctr_heap_reallocate(space, size);
	memBlocks[tracking_id].space = space;
	memBlocks[tracking_id].size = size;
	return space;
}

/**
 * Returns the latest tracking ID after tracking allocation.
 */
size_t ctr_heap_get_latest_tracking_id()
{
	return numberOfMemBlocks - 1;
}

/**
 * Frees all tracked memory blocks.
 */
void ctr_heap_free_rest()
{
	size_t i;
	for (i = 0; i < numberOfMemBlocks; i++) {
		ctr_heap_free(memBlocks[i].space);
	}
	//ctr_heap_free( memBlocks );
}

/**
 * Heap free memory
 *
 * Frees the memory pointed to by the specified pointer and deducts
 * the specified size from the allocation bookkeepting variable.
 *
 * @param void*     ptr  pointer to memory to be freed
 *
 * @return void
 */
void ctr_heap_free(void *ptr)
{
	// size_t *block_width;
	// int q = sizeof(size_t);
	// size_t size;

	// /* find the correct size of this memory block and move pointer back */
	// block_width = (size_t *) ((char *)ptr - q);
	// size = *block_width;
	// free((void *)block_width);
	// ctr_gc_alloc -= size;
	GC_FREE(ptr);
}

void ctr_heap_free_shared(void *ptr)
{

	size_t *block_width;
	int q = sizeof(size_t);
	size_t size;

	/* find the correct size of this memory block and move pointer back */
	ptr = (void *)((char *)ptr - q);
	block_width = (size_t *) ptr;
	size = *(block_width);

	munmap(ptr, size);	//so god help me
	ctr_gc_alloc -= size;
}

/**
 * Memory Management Adjust Memory Block Size (re-allocation)
 * Re-allocates Memory Block.
 *
 * Given the old pointer, the desired size, the original size and
 * the purpose for allocation, this function will attempt to
 * re-allocate the memory block.
 */
void *ctr_heap_reallocate(void *oldptr, size_t size)
{

	// char *nptr;
	// size_t old_size;
	// size_t *block_width;

	// /* correct the required size of the new block to include block width */
	// int q = sizeof(size_t);
	// size += q;
	// /* move the pointer back to begining of block */
	// oldptr = (void *)((char *)oldptr - q);
	// /* read memory size at beginning of old block */
	// block_width = (size_t *) oldptr;
	// old_size = *(block_width);

	// /* update the ledger */
	// ctr_gc_alloc = (ctr_gc_alloc - old_size) + size;
	// /* re-allocate memory */
	// nptr = realloc(oldptr, size);

	// /* store the size of the new block at the beginning */
	// block_width = (size_t *) nptr;
	// *(block_width) = size;
	// /* 'hop' the memory pointer over the block width part */
	// nptr = (void *)((char *)nptr + q);

	// return (void *)nptr;
	return GC_REALLOC(oldptr, size);
}

/**
 * @internal
 *
 * Casts a string object to a cstring.
 * Given an object with a string value, this function
 * will return a C-string representing the bytes contained
 * in the String Object. This function will explicitly end
 * the returned set of bytes with a \0 byte for use
 * in traditional C string functions.
 *
 * Warning: this function 'leaks' memory.
 * It will allocate the necessary resources to store the string.
 * To free this memory you'll need to call ctr_heap_free
 * passing the pointer and the number of bytes ( value.svalue->vlen ).
 *
 * @param ctr_object* stringObject CtrString object instance to cast
 *
 * @return char*
 */
char *ctr_heap_allocate_cstring(ctr_object * stringObject)
{
	char *cstring;
	char *stringBytes;
	ctr_size length;

	stringBytes = stringObject->value.svalue->value;
	length = stringObject->value.svalue->vlen;
	cstring = ctr_heap_allocate((length + 1) * sizeof(char));

	strncpy(cstring, stringBytes, length);
	cstring[length] = '\0';

	return cstring;
}

char *ctr_heap_allocate_cstring_shared(ctr_object * stringObject)
{
	char *cstring;
	char *stringBytes;
	ctr_size length;

	stringBytes = stringObject->value.svalue->value;
	length = stringObject->value.svalue->vlen;
	cstring = ctr_heap_allocate_shared((length + 1) * sizeof(char));

	strncpy(cstring, stringBytes, length);
	cstring[length] = '\0';

	return cstring;
}

void setup_types() {
	GC_word  bitmap[GC_BITMAP_SIZE(ctr_object)] = {0};
	GC_set_bit(bitmap, GC_WORD_OFFSET(ctr_object, properties));
	GC_set_bit(bitmap, GC_WORD_OFFSET(ctr_object, methods));
	GC_set_bit(bitmap, GC_WORD_OFFSET(ctr_object, link));
	GC_set_bit(bitmap, GC_WORD_OFFSET(ctr_object, lexical_name));
	GC_set_bit(bitmap, GC_WORD_OFFSET(ctr_object, release_hook));

	VT_descr = GC_make_descriptor(bitmap, GC_WORD_LEN(ctr_object));

	GC_word    bitmap_[GC_BITMAP_SIZE(ctr_object)] = {0};
	GC_set_bit(bitmap_, GC_WORD_OFFSET(ctr_object, properties));
	GC_set_bit(bitmap_, GC_WORD_OFFSET(ctr_object, methods));
	GC_set_bit(bitmap_, GC_WORD_OFFSET(ctr_object, link));
	GC_set_bit(bitmap_, GC_WORD_OFFSET(ctr_object, lexical_name));
	GC_set_bit(bitmap_, GC_WORD_OFFSET(ctr_object, release_hook));
	GC_set_bit(bitmap_, GC_WORD_OFFSET(ctr_object, value));

	PT_descr = GC_make_descriptor(bitmap_, GC_WORD_LEN(ctr_object));

	gc_types_initialized = 1;
}
