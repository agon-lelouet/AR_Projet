#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>

#include "conf.h"
#include "utils.h"

int m = M;

void simulateur(int size, int max_node) {
	int nodeid, not_uniq = 0;
	srand(getpid());
	struct node nodes[size];
	int chosenids[size];
	// génération de l'identifiant de chaque noeud
	for (int i = 0; i < size; i++) {
		do {
			nodeid = rand()%max_node;
			for (int j = 0; j < i; j++) {
				if (chosenids[j] == nodeid) {
					not_uniq = 1;
					break;
				}
			}
		} while (not_uniq);
		nodes[i].key = nodeid;
		nodes[i].rank = i+1;
	}
	printf("nodes created \n");

	// // we sort the nodes so fingers generation is more efficient
	sort(nodes, 0, size-1);
	printnodes(nodes, size, 0);
	//génération de la finger table de chaque noeud (à partir de la liste des noeuds)
	int start, startindex;
	struct node *currnode, *currfinger;
	for (int i = 0; i < size - 1; i++) {
		for (int j = 0; j < m; j++) {
			start = (nodes[i].key + (int)pow(2, j))% max_node;
			// printf("#####node %d finger %d start %d ####\n", i, j,start);
			// on parcourt du plus grand au plus petit noeud
			// donc le premier noeud dont l'index est supérieur à start est le finger correspondant
			int found = 0;
			for (int k = size-1; k >= 0; k--) {
				if (nodes[k].key <= start) {
					printf("%d next closest is %d \n", start, nodes[(k+1)%size].key);
					found = 1;
					nodes[i].fingers[j] = &nodes[(k+1)%size];
					break;
				}
			}
			if (!found) {
				nodes[i].fingers[j] = &nodes[0];
				printf("%d next closest is %d \n", start, nodes[0].key);
			}
		}
	}
	printnodes(nodes, size, 1);

	// NODES initialized, sendig data
	int mpi_dest;
	for (int i = 0; i < size; i++) {
		mpi_dest =  nodes[i].rank;
		// sending the CHORD node ID
		printf("sending CHORD ID table to %d", mpi_dest);
		MPI_Send(&nodes[i].key, 1, MPI_INT, mpi_dest, TAGINIT, MPI_COMM_WORLD);
		// // sending the finger table
		// for (int j = 0; j < M; j++) {
		// 	printf("sending finger table to %d", mpi_dest);
		// 	MPI_Send(nodes[i].fingers[j], sizeof(struct node), MPI_CHAR, mpi_dest, TAGINIT, MPI_COMM_WORLD);
		// }
	}
}

void node(int rank, int size) {
	struct node node = {
		.rank = rank,
		.fingers = (struct node *)malloc(M * sizeof(struct node))
	};
	MPI_Status status;
	printf("rank %d waiting for nodeid \n", rank);
	MPI_Recv(&node.key, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	printf("rank %d received nodeid %d \n", rank, node.key);
	// for (int i = 0; i < M; i++) {
	// 	MPI_Recv(&node.fingers[i], sizeof(struct node), MPI_CHAR, 0, TAGINIT, MPI_COMM_WORLD, &status);
	// }
	// printnodes(&node, 1, 1);
	return;
}

int main(int argc, char **argv) {
	int rank, size, max_node;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	max_node = pow(2, m);

	if (rank == 0) {
		simulateur(size, max_node);
	} else {
		node(rank, size);
	}
   	MPI_Finalize();
}