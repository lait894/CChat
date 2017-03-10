#include "list.h"
#include <stdlib.h>
#include <string.h>

void* listGet(List* list, int index)
{
	if (!list || index < 0) return NULL;
	if (index > list->size) return NULL;

	Node *curr = list->head;
	int i;
	for (i = 0; i < index; i++) {
		curr = curr->next;
	}

	return curr->item;
}

void* listHead(List* list)
{
	if (!list) return NULL;

	return list->head->item;
}

void* listTail(List* list)
{
	if (!list) return NULL;

	Node *curr = list->head;

	while(curr->next != NULL) {
		curr = curr->next;
	}

	return curr->item;
}

int listAdd(List* list, void* i)
{
	if (!list || !i) return -1;

	if (list->head == NULL) {
		list->head = malloc(sizeof(Node));
		list->head->item = i;
		list->head->next = NULL;
	} else {
		Node *curr;
		curr = list->head;

		while(curr->next != NULL) {
			curr = curr->next;
		}
		curr->next = malloc(sizeof(Node));

		curr = curr->next;
		if (!curr) return -2;
		curr->item = i;
		curr->next = NULL;
	}

	list->size++;
	return 0;
}

int listDel(List *list, void *i)
{
	if (!list || !i) return -1;
	if (list->head == NULL) return -2;

	Node *curr, *last;
	curr = last = list->head;

	while(curr != NULL) {
		if (curr->item == i) {
			if (last == curr) {
				list->head = curr->next;
			} else {
				last->next = curr->next;
			}

			free(curr->item);
			free(curr);
			list->size--;
			return 0;
		}
		last = curr;
		curr = curr->next;
	}

	return -2;
}

int listDelHead(List *list)
{
	if (!list) return -1;
	if (list->head == NULL) return 0;
	Node *target = list->head;
	list->head = list->head->next;
	free(target);
	list->size--;
	return 0;
}

// fix it later
int listDelTail(List *list)
{
	return listDel(list, listTail(list));
}

int listClear(List* list)
{
	if (!list) return -1;

	Node *curr, *tmp;
	curr = list->head;

	while(curr != NULL)
	{
		tmp = curr;
		free(tmp->item);
		free(tmp);

		curr = curr->next;
	}

	list->size = 0;

	return 0;
}

