#include "conf.h"

#ifndef EXO1UTILS
#define EXO1UTILS

void swap(struct node *node1, struct node *node2);

int partition(struct node *nodes, int low, int high);

void sort(struct node *nodes, int low, int high);

void printnodes(struct node *nodes, size_t size, int isfingers, int isreverse);

#endif