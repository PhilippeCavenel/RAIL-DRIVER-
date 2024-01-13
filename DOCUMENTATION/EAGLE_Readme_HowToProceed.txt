Points importants à retenir
---------------------------


Première étape : 
----------------

- Génération de la schématique puis validation du circuit avec un ERC
- Bien penser à la classe des nets notamment pour la partie puissance (power)

Deuxième étape : 
----------------

- Placement des composants et routage automatique. 
- Pour remplir le top et le bottom il faut faire un polygone Top et un polygone Bottom tout autour de la carte (draw a polygon) en sélectionnant en haut à gauche le bon layer (Top ou Bottom) après le routage (sinon ca ne passe pas) puis relancer le routage.
- Validation avec un DRC en ne sélectionnant que les couches Top, Bottom, Via et Pad
- Finalisation du silkscreen (couche 21) en plaçant toutes les indications correctement puis en générant la légende pour les drill sur la couche 144 à l'aide de drillegend-stack.ulp

Troisième étape : 
-----------------

- Générer les fichiers pour la fabrication avec le CAM Processor en cliquant sur le fichier CAM_JOB/RailDriverCamJob.cam
- Sélectionner dans le menu en haut à gauche Fichier le fichier .brd du design correspondant. 
- Les répertoires et nom de fichiers doivent être les bons dans les différents onglets (à vérifier et à corriger si nécessaire). 
- Il reste à lancer la fabrication des fichiers par PROCESS JOB

Quatrième étape : 
-----------------

- Pour modifier la sélection des boitiers inconnus il faut modifier le fichier 3D_RENDERING/eagle3d/ulp/3dusrpac.dat.  
- Mettre à jour le répertoire des fichiers de configuration dans 3dconf.dat 
- Générer un rendu 3D du design en lançant via l'icône ULP dans l'écran du layout 3D_RENDERING/eagle3d/ulp/3d50.ulp 
- Indiquer le fichier .brd ce qui va générer un fichier .pov qui pourra être traité par povray pour obtenir le rendu 3D. 
- Sélectionner "Modèle défini par l'utilisateur" dans l'onglet General 
- Ajouter le layer 25 dans Divers Dessin du boitier
- Modifier Ecriture situé sur la plaque pour n'afficher que les layer 21 et 25. 
- Ajouter les layer 25 et 27 dans Référence du boitier 
- Cliquer sur créer le fichier POVRay et Sortir. 
- Copier le fichier EAGLE_FILES/RailDriver/RailDriver.pov qui a été généré dans 3D_RENDERING/eagle3d/povray
- Refaire la même opération en déplaçant la caméra pour une prise en dessous (en jouant sur Y) et/ou sur les autres cotés (en jouant sur X et Z)
- Cliquer sur le fichier 3D_RENDERING/eagle3d/povray/RailDriver.pov pour lancer la fabrication de la vue 3D avec povray. Utiliser la résolution 800 x 600 AA 0.3 pour un temps de création d'image raisonnable, sinon pour une meilleure résolution : 1600 x 1200 A 0.3, mais le temps de fabrication de l'image prendra plusieurs dizaines de minutes.
- Sous eagle faire une copie écran des différents layer construits lors de la construction des fichiers de fabrication. placer toutes les vues dans PNGVIEW

COMMENT AJOUTER UN MODELE 3D : 
------------------------------

- Récupérer le modèle au format stl sur le net (snapeda par exemple). Si le modèle est un format step il faut le convertir avant en stp, par exemple sur https://polyd.com/fr/convertir-step-en-stl-en-ligne
- Utiliser l'utilitaire stl2pov.exe disponible dans utilities (répertoire installation eagle) (lancer un shell windows) stl2pov.exe myModel.stl > myModel.inc. Le copier dans 3D_RENDERING/eagle3d/povray. 

Par exemple : stl2pov.exe "TO220HeatSink\TO220_Heatsink.stp" > "TO220HeatSink\TO220HeatSink.inc"
G
- Modifier le fichier myModel.inc pour qu'il soit compris par povray, en prenant comme exemple : fk_222_sa.inc
- Ajouter dans le fichier 3D_RENDERING/eagle3d/povray/e3d_user.inc l'include vers ce fichier : #include "myModel.inc". 
- Ajouter dans le fichier 3D_RENDERING/eagle3d/ulp/3dusrpac.dat avec les informations suivantes :

Par exemple : FK222:0:1:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:HEATSINK_FK_222(:HEATSINK_FK_222_GRND


Signification de l'index

[00] Nom du package du composant de Eagle
[01] Output name
[02] Output value
[03] Define color bands
[04] SMD offset (parts will be moved pcb_cuhight up/down)
[05] LED options (The LED options dialog will be displayed)
[06] Ready for sockets (see explanation)
[07] Demande pour la hauteur d'un Quartz
[08] Has part a macro (for example, SMD jumpers)
[09] SMD resistor, Generate number combination
[10] Socket macro
[11] Height of socket in 1/10mm
[12] Comments concerning socket
[13] Internal for administration purpose (not used at present)
[14] Correction de rotation axe Y
[15] Offset de correction x
[16] Offset de correction y
[17] Offset de correction z
[18] Use Prefix from Part?
[19] Shunt on pinheader (a dialog will be displayed)
[20] Logo selection dialog will be shown
[21] Reserved[8]
[29] Bounding-Box Minimum
[30] Bounding-Box Maximum
[31] POV-Ray macro (Nom de la macro pov et une parenthèse gauche)
[32] Package comments (Allemand)
[33] Package comments (Anglais)

Cinquième étape : 
-----------------

- Mettre à jour la BOM pour l'achat des composants

Sixième étape :
---------------

-Compresser le répertoire CAM_JOB et PNGVIEW pour lancer la fabrication chez AllPcb : https://www.allpcb.com/ 




