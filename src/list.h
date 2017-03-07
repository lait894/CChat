#ifndef LIST_H
#define LIST_H

typedef struct ListNode {
	void* item;
	struct ListNode *next;
} Node;

typedef struct {
	Node* head;
	int size;
} List;

void* listGet(List *list, int index);
void* listHead(List *list);
void* listTail(List *list);
int listAdd(List *list, void *i);
int listDel(List *list, void *i);
int listDelHead(List *list);
int listDelTail(List *list);
int listClear(List* list);

#endif