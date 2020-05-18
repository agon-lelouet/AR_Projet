#ifndef EXO1CONFIG
#define EXO1CONFIG

#define M 6

#define TAGINIT 1
#define TAGQUERY 2
#define TAGOVER 3

#define successor(n) n.fingers[0]

struct node {
	int key; // la clef du noeud pour CHORD
	int rank; // l'identifiant physique du noeud
	struct node *fingers;
};

struct query {
	int key; // the key to search
	int reply_to; // the MPI node to send the answer to
};

#endif