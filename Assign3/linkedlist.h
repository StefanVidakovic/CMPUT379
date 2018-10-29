#include <stdlib.h>
#include <stdint.h>

struct node {
	uint32_t value;
	struct node *next;
};
void linkedlistPush(struct node *base, uint32_t value);
uint32_t linkedlistPop(struct node * base);