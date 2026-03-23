Un Dev Container (ou conteneur de développement) est un environnement de travail complet et préconfiguré qui s'exécute à l'intérieur d'un conteneur Docker. Au lieu d'installer manuellement des compilateurs, des bibliothèques (comme Qt6) ou des outils spécifiques sur votre propre ordinateur — avec les risques de conflits de versions que cela comporte — tout est encapsulé dans le conteneur.

Pour le développeur, l'expérience est transparente : on ouvre le projet dans VS Code, et l'éditeur se connecte automatiquement à l'intérieur du conteneur. Cela garantit que chaque membre de l'équipe travaille sur une réplique exacte de l'environnement de développement, éliminant définitivement le fameux problème du « ça marche sur ma machine, mais pas sur la tienne ».

Résumé de Configuration : DevContainer Qt6 / C++ / Arduino

1. Problématique Initiale
   Mise en place d'un environnement de développement isolé sous Docker (Ubuntu 22.04) pour un projet C++ utilisant Qt 6.8, nécessitant un accès aux ports série (Arduino), une interface graphique (X11) et des bibliothèques externes (MVS Hikrobot).

2. Obstacles Rencontrés et Solutions
   A. Version de Qt (Système vs SDK)
   Problème : apt install qt6-base-dev sur Ubuntu 22.04 installe la version 6.2.4, causant des erreurs de compilation (ambiguous overload dans QDebug) car le code source utilise des fonctionnalités de Qt 6.8.

Solution : Utilisation de aqtinstall pour installer la version exacte 6.8.2 dans /opt/Qt.

Note technique : L'architecture cible sur les serveurs Qt pour cette version est linux_gcc_64 (et non simplement gcc_64).

B. Configuration CMake
Problème : CMake ne localisait pas les fichiers de configuration Qt6.

Solution : Définition explicite de CMAKE_PREFIX_PATH pointant vers /opt/Qt/6.8.2/gcc_64/lib/cmake/Qt6 dans les presets.

C. Accès Matériel (Arduino)
Problème : Permissions refusées sur /dev/ttyUSB\* à l'intérieur du container.

Solution :

Ajout de --privileged et --group-add dialout dans runArgs.

Ajout de l'utilisateur vscode au groupe dialout via postCreateCommand.

D. Sécurité Git (Dubious Ownership)
Problème : FetchContent échouait lors du clone de dépendances (Catch2) car le dossier appartenait à un UID différent de l'hôte.

Solution : Exécution de git config --global --add safe.directory '\*' via postCreateCommand.

E. Interface Graphique
Problème : Impossible d'afficher l'interface Qt depuis le container.

Solution : Partage de la variable DISPLAY et montage du socket X11 (/tmp/.X11-unix) dans runArgs.
