# Arduino-Wemos-D1-MQTT-LCD-Screen
Affichage MQTT en boucle sur écran LCD 2 lignes de 16 caractères

A partir de JEEDOM ou directement en ligne de commande avec par exemple un client mosquitto les messages sont mémorisés dans un double tableau de variables puis affichés en boucle. 

Mode d'emploi :

Modifier les paramètres réseau dans le script
Modifier les topics pour ecouter les bon flux MQTT
Par défaut le flux écouté ou d'abonné est maison/aff/2x16A

Pour tester si cela fonctionne vous pouvez envoyer un message MQTT via 
le client mosquito par exemple avec la commande 

mosquitto_pub -h 192.168.0.66 -t "maison/aff/2x16A" -m  "{\"L1\":\"Contenu Ligne 1\",\"L2\":\"Contenu ligne 2\"}"

L'arduino suite à cette commande devrait réagir et afficher sur la ligne n°1 de l'écran le message : Contenu Ligne 1 et sur la ligne 2 le message : Contenu ligne 2

Un billet plus détaillé sur le fonctionnement devrait se compéter au fur et à mesure http://blog.myouaibe.com/index.php/post/Arduino-affichage-MQTT-sur-ecran-LCD-2x16



 
