#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "conf.h"
#include "utils.h"

int m = M;

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
		printf("node %d done \n", i);
		not_unique = 1;
	}

	// NODES initialized, sendig data
	int initiator;
	for (int i = 0; i < size; i++) {
		// sending the CHORD node ID
		printf("sending CHORD ID to %d \n", i);
		MPI_Send(&nodes[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
		initiator = rand()%2;
		MPI_Send(&initiator, 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
		printf("node %d initiator = %d \n", i, initiator);
	}
}

void node(int rank, int size, int max_node) {
	int initiator, found, mpi_succ, msg_source, key;
	MPI_Status status;
	struct node thisnode, fingers[M], *peers = (struct node *)malloc(size * sizeof(struct node));

	peers[rank].rank = rank;

	MPI_Recv(&key, 1, MPI_INT, size, TAGINIT, MPI_COMM_WORLD, &status);
	peers[rank].key = key;

	MPI_Recv(&initiator, 1, MPI_INT, size, TAGINIT, MPI_COMM_WORLD, &status);

	mpi_succ = (rank + 1) % size;
	printf("rank %d succ is %d \n", rank, mpi_succ);


	if (initiator) {
		printf("rank %d is initiator, sending to succ %d\n", rank, mpi_succ);
		MPI_Send(&rank, 1, MPI_INT, mpi_succ, TAGPEER, MPI_COMM_WORLD);
		MPI_Send(peers, size * (sizeof(struct node)), MPI_CHAR, mpi_succ, TAGPEER, MPI_COMM_WORLD);
	}

	while (1) {
		printf("rank %d waiting \n", rank);
		MPI_Recv(&msg_source, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		MPI_Recv(peers, size * (sizeof(struct node)), MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if (status.MPI_TAG == TAGPEER) {
			if (initiator && msg_source < rank) {
				printf("rank %d received message with lower rank %d, ignoring. \n", rank, msg_source);
			} else if (msg_source == rank) {
				printf("rank %d sending %d (all peers) to succ %d \n", rank, ALLPEERS, mpi_succ);
				MPI_Send(&msg_source, 1, MPI_INT, mpi_succ, ALLPEERS, MPI_COMM_WORLD);
				MPI_Send(peers, size * (sizeof(struct node)), MPI_CHAR, mpi_succ, ALLPEERS, MPI_COMM_WORLD);
				printf("rank %d sent %d (all peers) to succ %d \n", rank, ALLPEERS, mpi_succ);
			} else {
				printf("rank %d transmitting to %d \n", rank, mpi_succ);
				// on transmet
				peers[rank].rank = rank;
				peers[rank].key = key;
				MPI_Send(&msg_source, 1, MPI_INT, mpi_succ, TAGPEER, MPI_COMM_WORLD);
				MPI_Send(peers, size * (sizeof(struct node)), MPI_CHAR, mpi_succ, TAGPEER, MPI_COMM_WORLD);
				printf("rank %d transmitted to %d \n", rank, mpi_succ);
			}
		} else if (status.MPI_TAG == ALLPEERS) {
			printf("rank %d received ALLPEERS, terminating \n", rank);
			if (msg_source != rank) {
				MPI_Send(&msg_source, 1, MPI_INT, mpi_succ, ALLPEERS, MPI_COMM_WORLD);
				MPI_Send(peers, size * (sizeof(struct node)), MPI_CHAR, mpi_succ, ALLPEERS, MPI_COMM_WORLD);
			}
			break;
		} else {
			printf("wrong message type \n");
		}
	}
	//génération de la finger table de chaque noeud (à partir de la liste des noeuds)
	int start, startindex;
	struct node *currnode, *currfinger;

	sort(peers, 0,  size - 1);
	for (int j = 0; j < M; j++) {
		start = (key + (int)pow(2, j))% max_node;
		// printf("#####node %d finger %d start %d ####\n", i, j,start);
		// on parcourt du plus grand au plus petit noeud
		// donc le premier noeud dont l'index est supérieur à start est le finger correspondant
		int found = 0;
		for (int k = 0; k < size; k++) {
			// printf("start, %d, nodes[%d].key : %d \n", start, k, nodes[k].key);
			if (peers[k].key >= start) {
				// printf("%d next closest is %d \n", start, nodes[(k)%size].key);
				found = 1;
				fingers[j] = peers[k];
				break;
			}
		}
		if (!found) {
			fingers[j] = peers[0];
		}
	}
	thisnode.key = key;
	thisnode.rank = rank;
	for (int i = 0; i < M; i++) {
		thisnode.fingers[i] = &fingers[i];
	}

	if (rank == 0) {
		printf("rank %d node %d fingers : \n", thisnode.rank, thisnode.key);
		printnodes(&thisnode, 1, 1);
		MPI_Send(&key, 1, MPI_INT, mpi_succ, TAGINIT, MPI_COMM_WORLD);
	} else if (rank == size - 1) {
		MPI_Recv(&start, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		printf("rank %d node %d fingers : \n", thisnode.rank, thisnode.key);
		printnodes(&thisnode, 1, 1);
	} else {
		MPI_Recv(&start, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		printf("rank %d node %d fingers : \n", thisnode.rank, thisnode.key);
		printnodes(&thisnode, 1, 1);
		MPI_Send(&key, 1, MPI_INT, mpi_succ, TAGINIT, MPI_COMM_WORLD);
	}
	
	free(peers);
	return;
}

int main(int argc, char **argv) {
	int rank, size, max_node;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	max_node = pow(2, m);

	if (rank == size - 1) {
		simulateur(size - 1, max_node);
	} else {
		node(rank, size - 1, max_node);
	}
   	MPI_Finalize();
}