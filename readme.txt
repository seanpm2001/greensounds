====================================
           Greensound
       @ GRAME 2015-2016
        <fober@grame.fr>
====================================

------------------------------------
Ports de communication
------------------------------------
port 8000 	: Max -> Greensound
port 7001	: Greensound -> Max

------------------------------------
Messages générés par les capteurs
------------------------------------
/accelerometer	x y z
/rotation		x y z

------------------------------------
Messages générés par les contrôleurs
------------------------------------
/slider/1		val			de [0 à 1]
/slider/2		val			de [0 à 1]
/param/1		bool		[0 ou 1]
/param/2		bool		[0 ou 1]
/param/3		bool		[0 ou 1]
/mode			val 		[1 ou 2]

------------------------------------
Messages de contrôle
------------------------------------
/greensounds hello	ip	port	: message envoyé à l'initialisation
								  ip est l'adresse ip de l'application
								  port est le port d'écoute de l'application
								  en retour Max doit renvoyer le message '/greensound hello' (voir patch)
/greensounds ip connected		: message envoyé en accusé de réception du message '/greensound hello'
/greensounds version v			: numéro de version de l'application
								  le message est envoyé en réponse au message '/greensound version'

/greensound ip wait				: message envoyé toutes les secondes par un client qui vient de se connecter 
								  ou qui a reçu un message wait
/greensound ip play				: message envoyé toutes les secondes par un client qui a reçu un message play
/greensound ip bye				: message envoyé lorsqu'un client quitte

------------------------------------
Messages émis depuis max
------------------------------------
/greensound play
/greensound wait
/greensound quit				: force l'application cliente à quitter
/greensound/buttons n n n;		: état des boutons de contrôle (0 | 1)
/greensound seq n;				: passage à la séquence n


------------------------------------
 Historique des versions
------------------------------------
version 1.14
- nouveau message '/greensound seq' pour changement de séquence
- ajout de labels dans les boutons et en en-tête

version 1.13
- révision de l'interface utilisateur et marche forcée pour android active

version 1.12
- page d'erreur en espagnol

version 1.11
- valeurs inversées pour android

version 1.1
- capteurs de rotation remplacés par les accéléromètres

version 1.01
- changement bouton on 
- émission /mode 1 quand on passe en mode play 
- modification interface sliders (remontés) pour éviter conflit avec les modes
- modification interface : image pour les modes A et B

version 1.004
- changement d'icones + bouton off 

version 1.003
- version multi-touch 

version 1.002
- remplacement de l'image d'un slider 

version 1.001
- sliders initialisés en bas

version 1.00
- bug: fix numéro de slider incorrect

version 0.58
- révision de l'interface utilisateur
- changement de mode restreint à une zone autour du texte

version 0.57
- révision de l'interface utilisateur

version 0.56
- l'application quitte lorsqu'elle passe en mode veille

version 0.55
- correction de bug: version 0.54 compilée pour Moto E (i.e. sans rotation) et qui crashe en mode play

version 0.54
- spécialisation sur le capteur de rotation
- revision de l'interface utilisateur :
  suppression du panel de sélection des capteurs
  ajout de panels pour la gestion des erreurs

version 0.53
- émission des capteurs inhibée en mode 'wait'
- revision de l'interface utilisateur

version 0.52
- implémentation du protocole 'wait' 'play' avec un intervalle de 1s.
- support du message 'quit'
- emission du message 'bye' en sortie de l'application

version 0.51
- la page d'erreur permet de continuer

version 0.5
- version initiale conçue pour test
  aucune calibration des données n'est effectuée dans cette version
