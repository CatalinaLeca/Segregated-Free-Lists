#include "fc.h"

sfl_t *init_heap(int heap_start, unsigned int number_lists,
				 unsigned int bytes_per_list, int type)
{
	sfl_t *heap = (sfl_t *)malloc(sizeof(sfl_t));
	DIE(!heap, "Failed to allocate memory for segregated free lists");

	heap->sfl = (doubly_linked_list_t **)malloc(number_lists *
												sizeof(doubly_linked_list_t *));
	DIE(!heap->sfl, "Failed to allocate memory for sfl");

	heap->al = dll_create(sizeof(block_t));

	heap->number_lists = number_lists;
	heap->type = type;
	heap->total_alloc_bytes = 0;
	heap->total_free_bytes = number_lists * bytes_per_list;
	heap->total_malloc_calls = 0;
	heap->total_free_calls = 0;
	heap->total_fragmentations = 0;
	heap->start = heap_start;

	heap->size = number_lists * bytes_per_list;
	unsigned int num_bytes_per_list = 8, j;
	int current = heap_start;
	for (unsigned int i = 0; i < number_lists; i++) {
		heap->sfl[i] = dll_create(sizeof(block_t));
		block_t new_block;
		for (j = 0; j < bytes_per_list / num_bytes_per_list; j++) {
			new_block.address = current;
			new_block.size = num_bytes_per_list;
			new_block.data = NULL;
			dll_add_nth_node(heap->sfl[i], dll_get_size(heap->sfl[i]),
							 &new_block);
			current += num_bytes_per_list;
		}
		num_bytes_per_list <<= 1;
	}

	return heap;
}

void dump_memory(sfl_t *heap)
{
	unsigned int total_memory = heap->size;
	unsigned int total_allocated_memory = heap->total_alloc_bytes;
	unsigned int total_free_memory = heap->total_free_bytes;
	unsigned int num_free_blocks = 0;
	unsigned int num_allocated_blocks = 0;
	unsigned int num_malloc_calls = heap->total_malloc_calls;
	unsigned int num_fragmentations = heap->total_fragmentations;
	unsigned int num_free_calls = heap->total_free_calls;

	for (unsigned int i = 0; i < heap->number_lists; i++)
		num_free_blocks += dll_get_size(heap->sfl[i]);

	num_allocated_blocks = dll_get_size(heap->al);

	printf("+++++DUMP+++++\n");
	printf("Total memory: %u bytes\n", total_memory);
	printf("Total allocated memory: %u bytes\n", total_allocated_memory);
	printf("Total free memory: %u bytes\n", total_free_memory);
	printf("Free blocks: %u\n", num_free_blocks);
	printf("Number of allocated blocks: %u\n", num_allocated_blocks);
	printf("Number of malloc calls: %u\n", num_malloc_calls);
	printf("Number of fragmentations: %u\n", num_fragmentations);
	printf("Number of free calls: %u\n", num_free_calls);

	remove_size_zero_and_reallocate(heap);

	for (unsigned int i = 0; i < heap->number_lists; ++i) {
		unsigned int block_size = ((block_t *)heap->sfl[i]->head->data)->size;
		unsigned int num_free_blocks_of_size = dll_get_size(heap->sfl[i]);
		printf("Blocks with %u bytes - %u free block(s) :", block_size,
			   num_free_blocks_of_size);
		dll_node_t *current = heap->sfl[i]->head;
		while (current) {
			block_t *block = (block_t *)current->data;
			printf(" 0x%x", block->address);
			current = current->next;
		}
		printf("\n");
	}

	printf("Allocated blocks :");

	dll_node_t *current = heap->al->head;
	while (current) {
		block_t *block = (block_t *)current->data;
		printf(" (0x%x - %u)", block->address, block->size);
		current = current->next;
	}

	printf("\n");
	printf("-----DUMP-----\n");
}

void my_malloc(sfl_t *heap, unsigned int nr_bytes)
{
	unsigned int block_size;
	for (unsigned int i = 0; i < heap->number_lists; i++) {
		if (dll_get_size(heap->sfl[i]) > 0) {
			block_size = ((block_t *)heap->sfl[i]->head->data)->size;
			if (block_size >= nr_bytes) {
				dll_node_t *block_node = dll_remove_nth_node(heap->sfl[i], 0);
				block_t *block = (block_t *)block_node->data;
				if (block->size > nr_bytes) {
					block_t new_block = {.address = block->address + nr_bytes,
										 .size = block->size - nr_bytes};
					heap->total_fragmentations++;
					int index = find_list_by_size(heap, new_block.size);
					if (index != -1) {
						dll_insert_sorted(heap->sfl[index], &new_block);
					} else {
						insert_in_order(heap, heap->number_lists + 1,
										&new_block);
					}
					block->size = nr_bytes;
				}

				dll_insert_sorted(heap->al, block);

				heap->total_alloc_bytes += block->size;
				heap->total_free_bytes -= block->size;
				heap->total_malloc_calls++;
				free(block->data);
				free(block_node->data);
				free(block_node);
				return;
			}
		}
	}

	printf("Out of memory\n");
}

void my_free(sfl_t *heap, int address)
{
	if (address == 0)
		return;

	dll_node_t *current = heap->al->head;
	int k = 0;
	while (current) {
		if (((block_t *)current->data)->address == address) {
			dll_node_t *node = dll_remove_nth_node(heap->al, k);
			block_t *block = (block_t *)node->data;
			int index = find_list_by_size(heap, block->size);
			if (index != -1)
				dll_insert_sorted(heap->sfl[index], block);
			else
				insert_in_order(heap, heap->number_lists + 1, block);

			heap->total_alloc_bytes -= block->size;
			heap->total_free_bytes += block->size;
			heap->total_free_calls++;
			free(block->data);
			free(node->data);
			free(node);
			return;
		}
		current = current->next;
		k++;
	}

	printf("Invalid free\n");
}

void destroy_heap(sfl_t **heap)
{
	if ((*heap)->sfl)
		for (unsigned int i = 0; i < (*heap)->number_lists; ++i)
			dll_free(&(*heap)->sfl[i]);

	free((*heap)->sfl);

	dll_node_t *current = (*heap)->al->head;
	while (current) {
		block_t *block = (block_t *)current->data;
		free(block->data);
		block->data = NULL;
		current = current->next;
	}
	dll_free(&(*heap)->al);

	free(*heap);
	*heap = NULL;
}

void read_memory(sfl_t *heap, int address, unsigned int num_bytes)
{
	char *area = malloc(num_bytes + 1);
	int area_size = 0;
	int is_valid = 1;
	block_t *target_block = NULL;
	for (unsigned int i = 0; i < num_bytes; ++i) {
		unsigned int current_address = address + i;
		int found = 0;

		dll_node_t *current_node = heap->al->head;
		while (current_node) {
			block_t *block = (block_t *)current_node->data;
			if (current_address >= block->address &&
				current_address < block->address + block->size) {
				target_block = block;
				found = 1;
				break;
			}
			current_node = current_node->next;
		}

		if (!found) {
			is_valid = 0;
			break;
		}
		int offset = current_address - target_block->address;
		area[area_size] = ((char *)target_block->data)[offset];
		area_size++;
	}
	if (!is_valid) {
		printf("Segmentation fault (core dumped)\n");
		free(area);
		dump_memory(heap);
		destroy_heap(&heap);
		exit(0);
	}
	area[area_size] = '\0';
	printf("%s\n", area);
	free(area);
}

void write_memory(sfl_t *heap, int address, char *data, unsigned int num_bytes)
{
	int is_valid = 1;
	block_t *target_block = NULL;
	for (unsigned int i = 0; i < num_bytes; ++i) {
		unsigned int current_address = address + i;
		int found = 0;
		dll_node_t *current_node = heap->al->head;
		while (current_node) {
			block_t *block = (block_t *)current_node->data;
			if (current_address >= block->address &&
				current_address < block->address + block->size) {
				target_block = block;
				found = 1;
				break;
			}
			current_node = current_node->next;
		}

		if (!found) {
			is_valid = 0;
			break;
		}

		if (!target_block->data) {
			target_block->data = malloc(target_block->size * sizeof(char));
			DIE(!target_block->data, "Failed to allocate memory");
		}
		((char *)target_block->data)[current_address - target_block->address] =
			data[i];
	}
	if (!is_valid) {
		printf("Segmentation fault (core dumped)\n");
		dump_memory(heap);
		destroy_heap(&heap);
		exit(0);
	}
}

int main(void)
{
	sfl_t *heap;
	char heap_st[100];
	unsigned int heap_start, number_lists, bytes_per_list, type, number_bytes,
		address;
	char command[100];

	while (scanf("%s", command) == 1) {
		if (strcmp(command, "INIT_HEAP") == 0) {
			scanf("%s %u %u %d", heap_st, &number_lists, &bytes_per_list,
				  &type);
			heap_start = strtol(heap_st, NULL, 16);
			heap = init_heap(heap_start, number_lists, bytes_per_list, type);
		} else if (strcmp(command, "MALLOC") == 0) {
			scanf("%u", &number_bytes);
			my_malloc(heap, number_bytes);
		} else if (strcmp(command, "FREE") == 0) {
			scanf("%s", heap_st);
			address = strtol(heap_st, NULL, 16);
			my_free(heap, address);
		} else if (strcmp(command, "READ") == 0) {
			scanf("%x %u", &address, &number_bytes);
			read_memory(heap, address, number_bytes);
		} else if (strcmp(command, "WRITE") == 0) {
			char data[600], read_command[600];
			fgets(read_command, 600, stdin);
			char *start = strchr(read_command, '"');
			char *end = strrchr(read_command, '"');
			start[-1] = '\0';
			*end = '\0';
			strcpy(data, start + 1);
			number_bytes = atoi(end + 2);
			if (strlen(data) < number_bytes)
				number_bytes = strlen(data);

			address = strtol(read_command, NULL, 0);
			write_memory(heap, address, data, number_bytes);
		} else if (strcmp(command, "DUMP_MEMORY") == 0) {
			dump_memory(heap);
		} else if (strcmp(command, "DESTROY_HEAP") == 0) {
			destroy_heap(&heap);
			break;
		}
	}

	return 0;
}
