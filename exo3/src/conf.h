#ifndef EXO1CONFIG
#define EXO1CONFIG

#define M 6

#define TAGINIT 	1
#define TAGQUERY 	2
#define TAGOVER 	3

#define FIND 		4
#define UPDATEFINGERS 	5
#define SENDFINGERS 	6
#define SENDREVERSE 	7
#define ADDREVERSE	8
#define DELETEREVERSE	8

#define successor(n) n.fingers[0]

struct node {
	int key; // la clef du noeud pour CHORD
	int rank; // l'identifiant physique du noeud
	struct node *fingers;
	struct node *reverse;
	int reverse_number;
};

struct query {
	int type; // the query type
	int payload; // the payload
	int reply_to; // the MPI node to send the answer to
};

#endif