#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DIE(assertion, call_description)                 \
	do {                                                   \
		if (assertion) {                                     \
			fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__); \
			perror(call_description);                          \
			exit(errno);                                       \
		}                                                    \
	} while (0)

typedef struct dll_node_t {
	void *data;
	struct dll_node_t *prev, *next;
} dll_node_t;

typedef struct doubly_linked_list_t {
	dll_node_t *head;
	unsigned int data_size;
	unsigned int size;
} doubly_linked_list_t;

typedef struct block_t {
	int address;
	unsigned int size;
	void *data;
} block_t;

typedef struct {
	doubly_linked_list_t **sfl;
	doubly_linked_list_t *al;
	unsigned int number_lists;
	int type;
	unsigned int size;
	unsigned int start;
	unsigned int total_alloc_bytes;
	unsigned int total_free_bytes;
	unsigned int total_malloc_calls;
	unsigned int total_free_calls;
	unsigned int total_fragmentations;
} sfl_t;

doubly_linked_list_t *dll_create(unsigned int data_size);
dll_node_t *dll_get_nth_node(doubly_linked_list_t *list, unsigned int n);
void dll_add_nth_node(doubly_linked_list_t *list, unsigned int n,
					  const void *data);
dll_node_t *dll_remove_nth_node(doubly_linked_list_t *list, unsigned int n);
unsigned int dll_get_size(doubly_linked_list_t *list);
void dll_free(doubly_linked_list_t **pp_list);
void dll_insert_sorted(doubly_linked_list_t *list, const void *data);
void add_block_to_sfl(sfl_t *heap, block_t *new_block);
int find_list_by_size(sfl_t *heap, unsigned int size);
void insert_in_order(sfl_t *heap, unsigned int new_size, block_t *block);
void remove_size_zero_and_reallocate(sfl_t *heap);

#endif
