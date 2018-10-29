#include "linkedlist.h"

uint32_t linkedlistPop(struct node *base) {
	if (base->next == NULL) {
		return -1;
	}
	uint32_t first = base->next->value;
	struct node *ptr_to_be_freed = base->next;
	base->next = base->next->next;
	free(ptr_to_be_freed);
	return first;
}

void linkedlistPush(struct node *base, uint32_t value) {
	struct node *tempPtr = base;
	while (tempPtr->next != NULL) {
		tempPtr->next = tempPtr->next->next;
	}
	struct node *spaceCreatedPtr = malloc(sizeof(*base));
	tempPtr->next = spaceCreatedPtr;
	spaceCreatedPtr->value = value;
}