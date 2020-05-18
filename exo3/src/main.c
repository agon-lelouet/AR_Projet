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
	int nodeid, initsize = size - 1, not_unique = 1;
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
		if (i != initsize) {
			nodes[i].fingers = malloc(M * sizeof(struct node));
		}
		printf("node %d done \n", i);
		not_unique = 1;
	}
	// printf("nodes created \n");
	// printnodes(nodes, initsize, 0);
	// // we sort the nodes so fingers generation is more efficient
	sort(nodes, 0,  initsize - 1);
	printf("nodes sorted \n");
	printnodes(nodes, size, 0, 0);
	printf("##################\n");
	//génération de la finger table de chaque noeud (à partir de la liste des noeuds)
	int start, startindex;
	struct node *currnode, *currfinger;
	for (int i = 0; i < initsize; i++) {
		for (int j = 0; j < M; j++) {
			start = (nodes[i].key + (int)pow(2, j))% max_node;
			// printf("#####node %d finger %d start %d ####\n", i, j,start);
			// on parcourt du plus grand au plus petit noeud
			// donc le premier noeud dont l'index est supérieur à start est le finger correspondant
			int found = 0;
			for (int k = 0; k < initsize; k++) {
				// printf("start, %d, nodes[%d].key : %d \n", start, k, nodes[k].key);
				if (nodes[k].key >= start) {
					// printf("%d next closest is %d \n", start, nodes[(k)%initsize].key);
					found = 1;
					nodes[i].fingers[j].key = nodes[k].key;
					nodes[i].fingers[j].rank = nodes[k].rank;
					break;
				}
			}
			if (!found) {
				nodes[i].fingers[j] = nodes[0];
				// printf("%d next closest is %d \n", start, nodes[0].key);
			}
		}
	}


	//reverse tables
	int reverse_index;
	for (int i = 0; i < initsize; i++) {
		reverse_index = 0;
		nodes[i].reverse = malloc(initsize * sizeof(struct node));
		for (int j = 0; j < initsize; j++) {
			if (j == i) {
				continue;
			}
			for (int k = 0; k < M; k++) {
				if (nodes[j].fingers[k].key == nodes[i].key) {
					nodes[i].reverse[reverse_index].key = nodes[j].key;
					nodes[i].reverse[reverse_index].rank = nodes[j].rank;
					reverse_index++;
					break;
				}
			}
		}
		nodes[i].reverse_number = reverse_index;
	}
	printnodes(nodes, initsize, 1, 1);
	printf("#######################\n");

	// NODES initialized, sending data
	int mpi_dest;
	for (int i = 0; i < size; i++) {
		mpi_dest =  nodes[i].rank;
		// sending the CHORD node ID
		printf("sending CHORD ID to rank %d \n", mpi_dest);
		MPI_Send(&nodes[i], sizeof(struct node), MPI_CHAR, mpi_dest, TAGINIT, MPI_COMM_WORLD);

		if (i != initsize) {
			// sending the finger table
			printf("sending finger table to %d \n", mpi_dest);
			MPI_Send(nodes[i].fingers, M * sizeof(struct node), MPI_CHAR, mpi_dest, TAGINIT, MPI_COMM_WORLD);

			printf("sending reverse table to %d \n", mpi_dest);
			MPI_Send(&nodes[i].reverse_number, 1, MPI_INT, mpi_dest, TAGINIT, MPI_COMM_WORLD);

			MPI_Send(nodes[i].reverse, nodes[i].reverse_number * sizeof(struct node), MPI_CHAR, mpi_dest, TAGINIT, MPI_COMM_WORLD);
			printf("sent reverse table to %d \n", mpi_dest);
		}
	}
	do {
		mpi_dest = random()%size;
	} while (mpi_dest == initsize);
	printf("created node is node %d, creator node is rank %d \n", nodes[initsize].key, nodes[mpi_dest].key);
	MPI_Send(&nodes[mpi_dest], sizeof(struct node), MPI_CHAR, initsize, TAGINIT, MPI_COMM_WORLD);
	
	// int randkey, randnodeindex, res;
	// randkey = random()%max_node;
	// randnodeindex = (random()%size);
	// // printf("randomenodeindex = %d \n", randnodeindex);
	// struct query query = {
	// 	.key = randkey,
	// 	.reply_to = 0 
	// };
	// printf("*************sending query for key %d to node %d *************\n", query.key, nodes[randnodeindex].key);
	// MPI_Send(&query, sizeof(struct query), MPI_CHAR, nodes[randnodeindex].rank, TAGQUERY, MPI_COMM_WORLD
	// );
	// MPI_Recv(&res, 1, MPI_INT, MPI_ANY_SOURCE, TAGOVER, MPI_COMM_WORLD, &status);
	// printf("received response node %d from rank %d \n", res, status.MPI_SOURCE);
	for (int j = 0; j < initsize; j++) {
		free(nodes[j].fingers);
		free(nodes[j].reverse);
	}
	printf("CUL TERMINATING \n");
	return;
}

void creatednode(int rank, int size, int max_node) {
	int key;
	MPI_Status status;
	struct node node, received_node, *reverse, *fingers_succ, *fingers_pred;
	struct query query, answer;
	printf("created node waiting for nodeid \n");
	MPI_Recv(&node, sizeof(struct node), MPI_CHAR, size, TAGINIT, MPI_COMM_WORLD, &status);
	printf("created node waiting for creator node \n");
	MPI_Recv(&received_node, sizeof(struct node), MPI_CHAR, size, TAGINIT, MPI_COMM_WORLD, &status);

	node.fingers = malloc(M * sizeof(struct node));
	if (node.fingers == NULL) {
		printf("ERROR ! error when allocating memory for created node fingers \n");
		return;
	}

	printf("node %d sending FIND query to node %d \n", node.key, received_node.key);
	query.type = FIND;
	query.payload = node.key;
	query.reply_to = node.rank;

	MPI_Send(&query, sizeof(struct query), MPI_CHAR, received_node.rank, TAGQUERY, MPI_COMM_WORLD);

	MPI_Recv(&answer, sizeof(struct query), MPI_CHAR, MPI_ANY_SOURCE, TAGQUERY, MPI_COMM_WORLD, &status);

	printf("node %d received FIND answer : key %d \n", node.key, answer.payload);
	node.fingers[0].key = answer.payload;
	node.fingers[0].rank = answer.reply_to;

	reverse = malloc(size * sizeof(struct node));
	fingers_succ = malloc(M * sizeof(struct node));
	fingers_pred = malloc(M * sizeof(struct node));

	query.type = SENDREVERSE;
	MPI_Send(&query, sizeof(struct query), MPI_CHAR, node.fingers[0].rank, TAGQUERY, MPI_COMM_WORLD);
	MPI_Recv(reverse, size * sizeof(struct node), MPI_CHAR, node.fingers[0].rank, TAGQUERY, MPI_COMM_WORLD, &status);

	query.type = SENDFINGERS;
	MPI_Send(&query, sizeof(struct query), MPI_CHAR, node.fingers[0].rank, TAGQUERY, MPI_COMM_WORLD);
	MPI_Recv(fingers_succ, M * sizeof(struct node), MPI_CHAR, node.fingers[0].rank, TAGQUERY, MPI_COMM_WORLD, &status);

	// query.type = SENDFINGERS;
	// MPI_Send(&query, sizeof(struct query), MPI_CHAR, node.fingers[0].rank, TAGQUERY, MPI_COMM_WORLD);
	// MPI_Recv(reverse, size * sizeof(struct node), MPI_CHAR, node.fingers[0].rank, TAGQUERY, MPI_COMM_WORLD, &status);
	
	free(node.fingers);
	free(reverse);
	free(fingers_pred);
	free(fingers_succ);
}

void node(int rank, int size, int max_node) {
	struct node fingers[6], node;
	MPI_Status status;
	int found, payload, start, deletedkey, deletedrank;
	struct query query, answer;

	printf("rank %d waiting for nodeid \n", rank);
	MPI_Recv(&node, sizeof(struct node), MPI_CHAR, size, TAGINIT, MPI_COMM_WORLD, &status);
	printf("rank %d received nodeid %d \n", rank, node.key);
	node.fingers = malloc(M * sizeof(struct node));
	MPI_Recv(node.fingers, M * sizeof(struct node), MPI_CHAR, size, TAGINIT, MPI_COMM_WORLD, &status);
	printf("Rank %d received finger table \n", rank);

	MPI_Recv(&node.reverse_number, 1, MPI_INT, size, TAGINIT, MPI_COMM_WORLD, &status);

	node.reverse = malloc(size * sizeof(struct node));
	MPI_Recv(node.reverse, size * sizeof(struct node), MPI_CHAR, size, TAGINIT, MPI_COMM_WORLD, &status);
	printf("Rank %d received reverse \n", rank);

	// printnodes(&node, 1, 1);

	while (1) {
		MPI_Recv(&query, sizeof(struct query), MPI_CHAR, MPI_ANY_SOURCE, TAGQUERY, MPI_COMM_WORLD, &status);
		if (query.type == FIND) {
			found = 0;
			printf("node %d received query for key %d \n", node.key, query.payload);

			if (
			(node.key < node.fingers[0].key && query.payload > node.key && node.fingers[0].key >= query.payload)
			||
			node.key >= node.fingers[0].key && (query.payload > node.key || query.payload <= node.fingers[0].key)
			) {
				printf("#################### node %d FOUND ! %d ######################\n", node.key, node.fingers[0].key);
				answer.payload = node.fingers[0].key;
				answer.reply_to = node.fingers[0].rank;
				MPI_Send(&answer, sizeof(struct query), MPI_CHAR, query.reply_to, TAGQUERY, MPI_COMM_WORLD);
				continue;
			}
			printf("key %d not between %d and %d, looking on finger table... \n", query.payload, node.key, node.fingers[0].key);
			// sinon, on cherche le predecesseur d'id le plus élevé dans notre finger table
			int max = 0;
			for (int i = M-1; i >= 0; i--) {
				// node.key < node.fingers[i].key && 
				if (query.payload >= node.fingers[i].key) {
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
			// forwarding to highest fingers
			if (!found) {
				printf("key %d not found in finger table, sending to highest known node\n", query.payload);
				max = 0;
				for (int i = 0; i < M; i++) {
					if (node.fingers[i].key > node.fingers[max].key) {
						max = i;
					}
				}
			}
			printf("for node %d key %d highest preceding node is %d \n", node.key, query.payload, node.fingers[max].key);

			MPI_Send(&query, sizeof(struct query), MPI_CHAR, node.fingers[max].rank, TAGQUERY, MPI_COMM_WORLD);

		} else if (query.type == SENDFINGERS) {
			MPI_Send(node.fingers, M * sizeof(struct node), MPI_CHAR, query.reply_to, TAGQUERY, MPI_COMM_WORLD);
		} else if (query.type == SENDREVERSE) {
			MPI_Send(node.reverse, size * sizeof(struct node), MPI_CHAR, query.reply_to, TAGQUERY, MPI_COMM_WORLD);
		} else if (query.type == UPDATEFINGERS) {
			found = 0;
			for (int i = 0; i < M ; i++) {
				start = (node.key + (int)pow(2, i))% max_node;
				// si query.payload (l'ID du nouveau noeud qui pourrait être un finger) est compris entre le départ du finger actuel et l'identifiant CHORD du finger actuel (est donc un finger plus adapté).
				if (
					(start < node.fingers[i].key && query.payload > start && node.fingers[i].key >= query.payload)
					||
					start >= node.fingers[i].key && (query.payload > start || query.payload < node.fingers[i].key)
				) {
					found = 1;
					deletedkey = node.fingers[i].key;
					deletedrank = node.fingers[i].rank;
					node.fingers[i].key = query.payload;
					node.fingers[i].rank = query.reply_to;
				}
			}
			// si on a supprimé un finger, et qu'il n'est desormais plus dans la finger table, on en informe le noeud sur lequel il pointait afin que celui-ci puisse mettre à jour sa reverse table

			//Commenté pour faire des tests

			// if (found) {
			// 	found = 0;
			// 	for (int i = 0; i < M; i++) {
			// 		if (node.fingers[i].key == deletedkey) {
			// 			found = 1;
			// 			break;
			// 		}
			// 	}
			// 	if (!found) {
			// 		printf("sending DELETEREVERSE QUERY to %d \n", deletedkey);
			// 		query.type = DELETEREVERSE;
			// 		query.payload = deletedkey;
			// 		query.reply_to = node.rank;
			// 		MPI_Send(&query, sizeof(query), MPI_CHAR, deletedrank, TAGQUERY, MPI_COMM_WORLD);
			// 	}

			// }
		} else if (query.type == ADDREVERSE) {
			// Si on nous demande d'ajouter un élément dans la reverse table, on vérifie si cet élement n'est pas déjà présent, on l'insère si non, et on incrémente le nombre d'éléments dans cette table
			found = 0;
			for (int i = 0; i < node.reverse_number; i++) {
				if (node.reverse[i].key == query.payload) {
					found = 1;
					break;
				}
			}
			if (!found) {
				if (node.reverse_number <= size - 1) {
					node.reverse[node.reverse_number].key = query.payload;
					node.reverse[node.reverse_number].key = query.payload;
					node.reverse_number++;
				} else {
					printf("ERROR : trying to insert in reverse table with not enough space");
				}
			}
		} else if (query.type == DELETEREVERSE) {
			// Si on nous demande de supprimer un élément de la reverse table (un noeud arrète de pointer dessus), on enlève cet élément de la table des reverse, et on décrémente le nombre d'éléments dans cette table
			found = -1;
			for (int i = 0; i < node.reverse_number; i++) {
				if (node.reverse[i].key == query.payload) {
					found = i;
					break;
				}
			}
			if (found != -1) {
				for(int i = found; i < node.reverse_number - 1; i++) {
					node.reverse[i] = node.reverse[i + 1];
				}
				node.reverse_number--;
			} else {
				printf("ERROR : COULD NOT REMOVE key %d from node %d reverse array", query.payload, node.key);
			}
		}

		if (status.MPI_TAG == TAGOVER) {
			// printf("rank %d received over, terminating \n", rank);
			return;
		}
	}
	free(node.fingers);
	free(node.reverse);

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
	} else if (rank == size - 2) {
		creatednode(rank, size - 1, max_node);
	} else {
		node(rank, size - 1, max_node);
	}
   	MPI_Finalize();
}