Exercice 3 : insertion d'un pair

On fait les hypothèses suivantes: 
- Les canaux sont fiables, et FIFO.
- N'importe quel noeud peut communiquer avec n'importe quel autre noeud, sous reserve de connaitre son existance.
- Un noeud CHORD a les caractéristiques suivantes : Identifiant CHORD, Adresse physique, finger table (chaque entrée contenant l'identifiant CHORD et l'adresse physique du noeud concerné), reverse table (chaque entrée contenant l'identifiant CHORD et l'adresse physique du noeud concerné).

Soit N le nombre de noeuds du réseau. On considère que l'état global "conforme à la spécification" à atteindre après insertion du noeud est que le successeur et la finger table de chaque noeud soient corrects (comme précisé dans l'énoncé). Faire en sorte que les reverse tables soient correctes demandent l'envoi d'un plus grand nombre de messages, mais la démarche à suivre est quand même précisée dans l'algorithme qui suit, même si optionnelle.

Déroulement de l'algorithme : 

- Le noeud inséré ne connait qu'un seul noeud de la DHT CHORD. Il demande à celui-ci le successeur de sa clef, qui deviendra le successeur du noeud inséré. Si les finger tables sont correctement initialisées, cette recherche se fait en O(log(N)).
- Le noeud inséré demande la reverse table et la finger table de son successeur, et utilise la reverse table reçue pour trouver l'identifiant de son prédecesseur (le prédecesseur de son successeur) et lui demander sa finger table.
- Le noeud nouvellement inséré utilise les deux finger tables reçues afin d'obtenir une pseudo finger table susceptible d'accélerer la construction d'une finger table validée via l'envoi de messages de recherche de successeur. (et peut eventuellement envoyer log(N) messages pour mettre à jour les reverse finger tables des noeuds sur lesquels il pointe).
- Le noeud nouvellement crée indique à chaque reverse de son successeur (au pire N-1 messages), sa création, afin que ces noeuds puissent éventuellement mettre à jour leur finger table (les noeuds mettant à jour leur finger table peuvent par ailleurs mettre à jour les reverse tables des noeuds sur lesquels ils pointent, même si cela double la quantité de messages échangés).

Dans le pire des cas, cet algorithme a une complexité de O(N) (O(log(N) + N) = o(max(log(N), N)) =  O(N)). On peut néanmoins supposer que ce cas est rare, et que le nombre de messages à envoyer en moyenne est inférieur à N, donnant une complexité moyenne logiquement inférieure à N.

Notons que la complexité moyenne ou au pire cas reste la même qu'on mette à jour les reverse tables ou non, puisque N ou 2N sont toutes les deux O(N).

Pour ce qui est de l'implémentation de cet exercice, seule la recherche du successeur du noeud inséré est effectivement fonctionnelle, par manque de temps.

Afin de tester ce code, mettre un nombre de processus MPI égal au nombre de pairs CHORD souhaité + 1 (pour le simulateur).