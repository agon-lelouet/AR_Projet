#ifndef EXO1CONFIG
#define EXO1CONFIG

#define M 6

#define TAGINIT 1

#define successor(n) n.fingers[0]

struct node {
	int key; // la clef du noeud pour CHORD
	int rank; // l'identifiant physique du noeud
	struct node* fingers[M];
};

#endif