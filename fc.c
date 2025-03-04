#include "fc.h"

doubly_linked_list_t *dll_create(unsigned int data_size)
{
	doubly_linked_list_t *list = (doubly_linked_list_t *)malloc(sizeof(*list));
	DIE(!list, "Malloc failed - creating the list\n");
	list->head = NULL;
	list->data_size = data_size;
	list->size = 0;

	return list;
}

dll_node_t *dll_get_nth_node(doubly_linked_list_t *list, unsigned int n)
{
	if (!list || !list->head)
		return NULL;

	dll_node_t *tmp = list->head;

	for (unsigned int i = 0; i < n; i++) {
		if (!tmp->next)
			return tmp;
		tmp = tmp->next;
	}

	return tmp;
}

void dll_add_nth_node(doubly_linked_list_t *list, unsigned int n,
					  const void *data)
{
	if (!list)
		return;

	dll_node_t *new = (dll_node_t *)malloc(sizeof(*new));
	DIE(!new, "malloc new_node");
	new->data = malloc(list->data_size);
	DIE(!new->data, "malloc new_node->data");
	memcpy(new->data, data, list->data_size);

	if (!list->head) {
		new->next = NULL;
		new->prev = NULL;
		list->head = new;
		list->size++;
		return;
	}

	if (n == 0) {
		new->next = list->head;
		new->prev = NULL;
		list->head->prev = new;
		list->head = new;
		list->size++;
		return;
	}

	if (n > list->size)
		n = list->size;

	dll_node_t *tmp = list->head;

	for (unsigned int i = 0; i < n - 1; i++)
		tmp = tmp->next;

	new->prev = tmp;
	new->next = tmp->next;
	if (tmp->next)
		tmp->next->prev = new;

	tmp->next = new;
	list->size++;
}

dll_node_t *dll_remove_nth_node(doubly_linked_list_t *list, unsigned int n)
{
	if (!list || !list->head)
		return NULL;

	dll_node_t *removed = NULL;

	if (n == 0) {
		removed = list->head;
		list->head = list->head->next;
		if (list->head)
			list->head->prev = NULL;
		list->size--;
		return removed;
	}

	dll_node_t *tmp = list->head;

	if (n > list->size - 1)
		n = list->size - 1;

	for (unsigned int i = 0; i < n; i++)
		tmp = tmp->next;

	removed = tmp;
	tmp->prev->next = tmp->next;
	if (tmp->next)
		tmp->next->prev = tmp->prev;

	list->size--;
	return removed;
}

unsigned int dll_get_size(doubly_linked_list_t *list)
{
	if (!list || !list->head)
		return 0;

	return list->size;
}

void dll_free(doubly_linked_list_t **pp_list)
{
	if (!pp_list || !*pp_list)
		return;

	dll_node_t *tmp = (*pp_list)->head;
	dll_node_t *next;
	while (tmp) {
		next = tmp->next;
		free(tmp->data);
		free(tmp);
		tmp = next;
	}
	free(*pp_list);

	*pp_list = NULL;
}

void dll_insert_sorted(doubly_linked_list_t *list, const void *data)
{
	if (!list || !data)
		return;

	unsigned int position = 0;
	dll_node_t *current = list->head;
	while (current && *((int *)current->data) < *((int *)data)) {
		current = current->next;
		position++;
	}

	dll_add_nth_node(list, position, data);
}

void add_block_to_sfl(sfl_t *heap, block_t *new_block)
{
	unsigned int block_size = new_block->size;
	unsigned int size_class = 0;
	while (block_size > 8) {
		block_size >>= 1;
		size_class++;
	}

	dll_node_t *current = heap->sfl[size_class]->head;
	while (current) {
		block_t *block = (block_t *)current->data;
		if (new_block->address < block->address) {
			dll_add_nth_node(heap->sfl[size_class],
							 dll_get_size(heap->sfl[size_class]), new_block);
			return;
		}
		current = current->next;
	}

	dll_add_nth_node(heap->sfl[size_class], dll_get_size(heap->sfl[size_class]),
					 new_block);
}

int find_list_by_size(sfl_t *heap, unsigned int size)
{
	for (unsigned int i = 0; i < heap->number_lists; i++) {
		if (!heap->sfl[i]->head)
			continue;
		int block_size = ((block_t *)heap->sfl[i]->head->data)->size;
		if (block_size == size)
			return i;
	}
	return -1;
}

void insert_in_order(sfl_t *heap, unsigned int new_size, block_t *block)
{
	heap->sfl = realloc(heap->sfl, new_size * sizeof(doubly_linked_list_t *));
	DIE(!heap->sfl, "Failed to allocate memory for sfl");

	int list_place = 0;
	while (list_place < heap->number_lists &&
		   (!heap->sfl[list_place]->head ||
			block->size > ((block_t *)heap->sfl[list_place]->head->data)->size))
		list_place++;

	for (int i = heap->number_lists - 1; i >= list_place; i--)
		heap->sfl[i + 1] = heap->sfl[i];

	heap->sfl[list_place] = dll_create(sizeof(block_t));

	dll_add_nth_node(heap->sfl[list_place], 0, block);
	heap->number_lists = new_size;
}

void remove_size_zero_and_reallocate(sfl_t *heap)
{
	unsigned int cnt_non_zero = 0;
	for (unsigned int i = 0; i < heap->number_lists; i++)
		if (dll_get_size(heap->sfl[i]) > 0)
			cnt_non_zero++;

	doubly_linked_list_t **new_sfl = malloc(cnt_non_zero * sizeof(**new_sfl));

	if (!new_sfl)
		return;

	unsigned int j = 0;
	for (unsigned int i = 0; i < heap->number_lists; i++) {
		if (dll_get_size(heap->sfl[i]) > 0)
			new_sfl[j++] = heap->sfl[i];
		else
			dll_free(&heap->sfl[i]);
	}

	free(heap->sfl);

	heap->sfl = new_sfl;
	heap->number_lists = cnt_non_zero;
}
