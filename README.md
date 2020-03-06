IA-2_SteerBehavior

Projet basé sur le chapitre 3 du livre de Mat Buckland "Programming Game AI by Example" (http://www.ai-junkie.com/books/toc_pgaibe.html).


Ajout effectué :

- Le projet a des méthodes (CreateWithPursuitAgent, CreateLeaderPlayerWithAgent) pour créer la situation du leader 
avec des agents poursuite (PursuitAgent), un leader guidé par un joueur protégé par des agents et poursuivi par d'autres.

- Chaque type d'agent possède une couleur particulière (Rouge : Leader, Bleu : ProtectAgent, Vert : PursuitAgent).

- Ajout de menu avec la possibilité de modifier l'offset des agents (augmenter ou diminuer), d'ajouter un agent poursuite 
et de choisir la situation entre le leader normal et le leader guidé par le joueur. Chaque option à une touche associée.
Les nouveaux menus sont ajoutés dans les fichiers ressource.h, Script1.rc. Ils sont ajoutés aux méthodes HandleKeyPress 
et HandleMenuItems

- Les fichiers params.ini et ParamLoader.cpp ont été modifiés pour ajouter les données suivantes :
NumAgentProtectLeader, LeaderScale, LeaderMaxSpeed, LeaderPlayerMaxSpeed, ProtectLeaderMaxSpeed, OffsetAgent et ChangeOffset.

- Pour le Leader guidé par le joueur, j'ai ajouté un contrôle des murs pour la position de la souris. Ainsi, le joueur ne peut 
pas sélectionner les zones extérieures aux murs quand ils sont présents. Sinon le leader se bloque au mur. Cet ajout se fait via 
la variable m_vPointsWall dans la fonction SetCrossHair du fichier GameWorld.cpp.


En cas de problème sur le SDK :

Propriétés de la solution < Général
Adapter la "Version du Windows SDK" et "Ensemble d'outils de plateforme" aux versions que vous utilisez.