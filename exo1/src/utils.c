#include <stdlib.h>
#include <stdio.h>

#include "utils.h"

void swap(struct node *node1, struct node *node2) {
	struct node temp = *node1;
	*node1 = *node2;
	*node2 = temp;
}

int partition(struct node *nodes, int low, int high) {
	int pivot = nodes[high].key;
	int i = low - 1;

	for (int j = low; j <= high - 1; j++) {
		if (nodes[j].key < pivot) {
			i++;
			swap(&nodes[i], &nodes[j]);
		}
	}
	swap(&nodes[i + 1], &nodes[high]);
	return i+1;
}

void sort(struct node *nodes, int low, int high) {
	if (low < high) {
		int pi = partition(nodes, low, high);
		sort(nodes, low, pi - 1);
		sort(nodes, pi+1, high);
	}
}

void printnodes(struct node *nodes, size_t size, int isfingers) {
	for (int i = 0; i < size; i++) {
		printf("#### chord node : %d, mpi rank %d, fingers : #### \n", nodes[i].key, nodes[i].rank);
		if (isfingers) {
			for (int j = 0; j < M; j++) {
				printf("finger N°%d : chord node %d mpi rank %d \n", j, nodes[i].fingers[j]->key, nodes[i].fingers[j]->rank);
			}
		}
	}

}