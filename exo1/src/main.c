#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "conf.h"
#include "utils.h"

void simulateur(int size, int max_node) {
	int nodeid, not_unique = 1;
	struct node nodes[size];
	int chosenids[size];
	memset(chosenids, 0, size * sizeof(int));
	MPI_Status status;

	srand(getpid());
	// génération de l'identifiant de chaque noeud
	printf("creating nodes \n");
	for (int i = 0; i < size; i++) {
		while(not_unique) {
			nodeid = rand()%max_node;
			not_unique = 0;
			for (int j = 0; j < size; j++) {
				if (chosenids[j] == nodeid) {
					not_unique = 1;
				}
			}
		}
		nodes[i].key = nodeid;
		nodes[i].rank = i;
		nodes[i].fingers = malloc(M * sizeof(struct node));
		printf("node %d done \n", i);
		not_unique = 1;
	}
	// printf("nodes created \n");
	// printnodes(nodes, size, 0);
	// // we sort the nodes so fingers generation is more efficient
	sort(nodes, 0,  size - 1);
	printf("nodes sorted \n");
	printnodes(nodes, size, 0);
	//génération de la finger table de chaque noeud (à partir de la liste des noeuds)
	int start, startindex;
	struct node *currnode, *currfinger;
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < M; j++) {
			start = (nodes[i].key + (int)pow(2, j))% max_node;
			// printf("#####node %d finger %d start %d ####\n", i, j,start);
			// on parcourt du plus grand au plus petit noeud
			// donc le premier noeud dont l'index est supérieur à start est le finger correspondant
			int found = 0;
			for (int k = 0; k < size; k++) {
				// printf("start, %d, nodes[%d].key : %d \n", start, k, nodes[k].key);
				if (nodes[k].key >= start) {
					// printf("%d next closest is %d \n", start, nodes[(k)%size].key);
					found = 1;
					nodes[i].fingers[j] = nodes[k];
					break;
				}
			}
			if (!found) {
				nodes[i].fingers[j] = nodes[0];
				// printf("%d next closest is %d \n", start, nodes[0].key);
			}
		}
	}
	printnodes(nodes, size, 1);

	// NODES initialized, sendig data
	int mpi_dest;
	for (int i = 0; i < size; i++) {
		mpi_dest =  nodes[i].rank;
		// sending the CHORD node ID
		// printf("sending CHORD ID to %d \n", mpi_dest);
		MPI_Send(&nodes[i].key, 1, MPI_INT, mpi_dest, TAGINIT, MPI_COMM_WORLD);
		// sending the finger table
		// printf("sending finger table to %d \n", mpi_dest);
		MPI_Send(nodes[i].fingers, M * sizeof(struct node), MPI_CHAR, mpi_dest, TAGINIT, MPI_COMM_WORLD);
	}
	int randkey, randnodeindex, res;
	randkey = random()%max_node;
	randnodeindex = (random()%size);
	// printf("randomenodeindex = %d \n", randnodeindex);
	struct query query = {
		.key = randkey,
		.reply_to = 0 
	};
	printf("*************sending query for key %d to node %d *************\n", query.key, nodes[randnodeindex].key);
	MPI_Send(&query, sizeof(struct query), MPI_CHAR, nodes[randnodeindex].rank, TAGQUERY, MPI_COMM_WORLD
	);
	MPI_Recv(&res, 1, MPI_INT, MPI_ANY_SOURCE, TAGOVER, MPI_COMM_WORLD, &status);
	printf("received response node %d from rank %d \n", res, status.MPI_SOURCE);
	for (int j = 0; j < size; j++) {
		MPI_Send(&res, 1, MPI_INT, nodes[j].rank, TAGOVER, MPI_COMM_WORLD);
		free(nodes[j].fingers);
	}
	printf("SENT TAGOVER TO EVERYONE, TERMINATING \n");
	return;
}

void node(int rank, int size) {
	struct node fingers[6];
	struct node node = {
		.rank = rank
	};
	MPI_Status status;
	// printf("rank %d waiting for nodeid \n", rank);
	MPI_Recv(&node.key, 1, MPI_INT, size, TAGINIT, MPI_COMM_WORLD, &status);
	// printf("rank %d received nodeid %d \n", rank, node.key);
	node.fingers = malloc(M * sizeof(struct node));
	MPI_Recv(node.fingers, M * sizeof(struct node), MPI_CHAR, size, TAGINIT, MPI_COMM_WORLD, &status);
	// printf("Rank %d received finger table \n", rank);
	// printnodes(&node, 1, 1);
	int found;
	while (1) {
		struct query query;
		MPI_Recv(&query, sizeof(struct query), MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		found = 0;

		if (status.MPI_TAG == TAGOVER) {
			// printf("rank %d received over, terminating \n", rank);
			return;
		}

		printf("node %d received query for key %d \n", node.key, query.key);
		// on vérifie si la clef reçue est entre notre clef est la clef de notre successeur
		if (
			(node.key < node.fingers[0].key && query.key > node.key && node.fingers[0].key >= query.key)
			||
			node.key >= node.fingers[0].key && (query.key > node.key || query.key <= node.fingers[0].key)
		) {
			printf("node %d FOUND ! %d", node.key, node.fingers[0].key);
			MPI_Send(&node.fingers[0].key, 1, MPI_INT, size, TAGOVER, MPI_COMM_WORLD);
			continue;
		}
		printf("key %d not between %d and %d, looking on finger table... \n", query.key, node.key, node.fingers[0].key);
		int max = 0;
		for (int i = M-1; i >= 0; i--) {
			// node.key < node.fingers[i].key && 
			if (query.key >= node.fingers[i].key) {
				if (found) {
					if (node.fingers[i].key > node.fingers[max].key) {
						max = i;
					}
				} else {
					max = i;
					found = 1;
				}
			}
		}
		// sinon, on cherche le predecesseur d'id le plus élevé dans notre finger table
		if (!found) {
			printf("key %d not found in finger table, sending to highest known node\n", query.key);
			max = 0;
			for (int i = 0; i < M; i++) {
				if (node.fingers[i].key > node.fingers[max].key) {
					max = i;
				}
			}
		}
		printf("for node %d key %d highest preceding node is %d \n", node.key, query.key, node.fingers[max].key);
		MPI_Send(&query, sizeof(struct query), MPI_CHAR, node.fingers[max].rank, TAGQUERY, MPI_COMM_WORLD);
	}

	return;
}

int main(int argc, char **argv) {
	int rank, size, max_node;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	max_node = pow(2, M);

	if (rank == size - 1) {
		simulateur(size - 1, max_node);
	} else {
		node(rank, size - 1);
	}
   	MPI_Finalize();
}