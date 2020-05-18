On implémente l'algorithme de chang roberts sur un anneau unidirectionnel possedant une ordre total sur les identifiants de noeud, des canaux unidirectionnels fiables et FIFO.

Chaque noeud possède : identifiant de noeud sur l'anneau logique de chang roberts, un identifiant CHORD, et une adresse physique.
On peut avoir plusieurs initiateurs.

Chaque noeud initiateur envoie à son successeur un message contenant son adresse physique et son identifiant CHORD, encapsulé dans un "jeton" comportant son identifiant de noeud sur l'anneau logique.

Un noeud non initiateur recevant un tel message ne peut plus devenir initiateur, ajoute son rang MPI et son identifiant de noeud à la liste, puis le transmet à son voisin (sans modifier l'identifiant de noeud porté par le jeton) et ainsi de suite.

Si un noeud initiateur reçoit un message, il compare l'identifiant de noeud porté par le jeton avec le sien : 
- si son identifiant est inférieur : il transmet le message en ajoutant ses informations, sans modifier l'identifiant de noeud porté par le jeton.
- si son identifiant est supérieur : il ne transmet pas le message, ce qui permet de limiter le nombre de messages en transition sur l'anneau.
- si son identifiant est égal : il a reçu son propre jeton, qui a donc fait le tour de l'anneau, le message contient donc la liste de tous les pairs CHORD, il le transmet via un message spécial à son successeur, ce qui donnera aux autres noeuds de l'anneau la liste complète des pairs CHORD, tous les noeuds pourront donc créer leur finger table à partir de cette liste.

Afin de tester ce code, mettre un nombre de processus MPI égal au nombre de pairs CHORD souhaité + 1 (pour le simulateur).