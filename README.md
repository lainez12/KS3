# KUB3 Software Suite
***(Version française en dessous)***

*One software to rule them all, One software to find them, One software to bring them all, and in the source code bind them;*  
*In the Land of Kloe where the performance lie.*

This repository contains the unified codebase for the KUB3 product line (4", 6", and 8" models). The project is architected for a 15+ year lifecycle using **C++20**, **Qt 6.8 LTS**, and **CMake**.

## 1. Prerequisites
Before building, ensure your development environment is prepared:
*   **Compiler:** GCC 13+ or equivalent (supporting C++20).
*   **Framework:** [Qt 6.8 LTS](https://www.qt.io/) installed.
*   **Vision SDK:** [Hikrobot MVS SDK](https://www.hikrobotics.com/) installed at `/opt/MVS`.
*   **Build Tools:** CMake 3.20+, Make/Ninja.

## 2. First-Time Setup
If you have just cloned this repository, follow these steps to initialize your environment:

1.  **Initialize Presets:** We use `CMakePresets.json` for reproducible builds. If this file is missing from your root directory, create it from the template:
    ```bash
    cp CMakePresets.json.template CMakePresets.json
    ```
2.  **Permissions (Required for Release Mode):** The software logs to system directories in Release mode. Prepare the log directory:
    ```bash
    sudo mkdir -p /var/log/kub3
    sudo chown $USER:$USER /var/log/kub3
    ```

## 3. The Build System (CMake Presets)
We use **CMake Presets** to handle conditional compilation (e.g., selecting the 4", 6", or 8" model). 
*   **VS Code:** The CMake Tools extension will automatically detect the presets. Go to the CMake extension tab and select a preset to choose your model and build type.
*   **Qt Creator:** Select the preset from the "Projects" -> "Build & Run" menu.

**Manual Build (Terminal):**
```bash
# Configure for 8" Machine in Debug mode
cmake --preset kub3-8-debug

# Build
cmake --build --preset kub3-8-debug
```

## 4. Versioning Workflow
We do **not** manually edit the version file. Use the automation script located in the root directory:
```bash
./version-bump.sh
```
This script will prompt you to choose between Major, Minor, Patch, or Build updates and automatically reset lower-significance numbers. **Always run this before a release cycle.**

## 5. Logging Strategy
The application uses a unified `Logger` class. It automatically handles file rotation and pathing based on the build type:
*   **Debug Mode:** Logs are saved to `./logs/kub3_YYYY-MM-DD.log` (Local to the build folder).
*   **Release Mode:** Logs are saved to `/var/log/kub3/kub3_YYYY-MM-DD.log` (System-wide).

## 6. Architecture & Logic
The project is strictly separated into layers to ensure testability:
*   **`Kub3Core` (Static Library):** Contains the "Brain"—HAL (Hardware Abstraction Layer), FSM (Finite State Machine), and Services. **All business logic must live here.**
*   **`Kub3Software` (Executable):** Contains the "Body"—UI, ViewModels, and main entry point. 
*   **`tests/`:** Contains unit tests using [Catch2](https://github.com/catchorg/Catch2).

## 7. Testing
We use **Catch2** for logic testing.
*   **Run via Terminal:**
    ```bash
    cd build/<your-preset-dir>
    ctest --output-on-failure
    ```
*   **Run via VS Code:** Use the "Testing" (beaker icon) tab in the sidebar.

## 8. Troubleshooting FAQ
*   **"Undefined reference to vtable":** You likely added a class with `Q_OBJECT` but forgot to add the header file to the `qt_add_library` or `qt_add_executable` list in `CMakeLists.txt`. Ensure all files are registered.
*   **"MVS Lib not found":** Ensure the Hikrobot MVS SDK is installed in `/opt/MVS`. If you are developing on a machine without a camera, the build will issue a warning but will still compile with vision features disabled.
*   **Linker Error `cannot find -lKub3Core`:** You likely renamed the library. Ensure the target name in `CMakeLists.txt` matches exactly what you are linking in `target_link_libraries`.

***
*For further architectural questions, consult the `include/` directory structure which maps to the logical layers of the KUB3 state machine.*

<br>
<br>
<br>

# Suite Logicielle KUB3

*Un logiciel pour les gouverner tous, un logiciel pour les trouver, un logiciel pour les amener tous, et dans le code source les lier ;*  
*Au pays de Kloe où s'étendent les performances.*

Ce dépôt contient la base de code unifiée pour la gamme de produits KUB3 (modèles 4", 6" et 8"). Le projet est conçu pour un cycle de vie de 15 ans et plus, utilisant **C++20**, **Qt 6.8 LTS** et **CMake**.

## 1. Prérequis
Avant de compiler, assurez-vous que votre environnement de développement est prêt :
*   **Compilateur :** GCC 13+ ou équivalent (supportant C++20).
*   **Framework :** [Qt 6.8 LTS](https://www.qt.io/) installé.
*   **SDK de Vision :** [Hikrobot MVS SDK](https://www.hikrobotics.com/) installé dans `/opt/MVS`.
*   **Outils de build :** CMake 3.20+, Make ou Ninja.

## 2. Configuration initiale
Si vous venez de cloner ce dépôt, suivez ces étapes pour initialiser votre environnement :

1.  **Initialiser les Presets :** Nous utilisons `CMakePresets.json` pour des builds reproductibles. Si ce fichier est absent de votre répertoire racine, créez-le à partir du modèle :
    ```bash
    cp CMakePresets.json.template CMakePresets.json
    ```
2.  **Permissions (Requis pour le mode Release) :** Le logiciel enregistre les logs dans des répertoires système en mode Release. Préparez le répertoire de log :
    ```bash
    sudo mkdir -p /var/log/kub3
    sudo chown $USER:$USER /var/log/kub3
    ```

## 3. Le système de compilation (Presets CMake)
Nous utilisons les **Presets CMake** pour gérer la compilation conditionnelle (ex: sélectionner le modèle 4", 6" ou 8").
*   **VS Code :** L'extension CMake Tools détectera automatiquement les presets. Allez dans l'onglet de l'extension CMake et sélectionnez un preset pour choisir votre modèle et votre type de build.
*   **Qt Creator :** Sélectionnez le preset depuis le menu "Projets" -> "Build & Run".

**Compilation manuelle (Terminal) :**
```bash
# Configurer pour une machine 8" en mode Debug
cmake --preset kub3-8-debug

# Compiler
cmake --build --preset kub3-8-debug
```

## 4. Flux de travail de versionnage
Nous ne modifions **jamais** le fichier de version manuellement. Utilisez le script d'automatisation situé à la racine :
```bash
./version-bump.sh
```
Ce script vous demandera de choisir entre une mise à jour Majeure, Mineure, Patch ou Build et réinitialisera automatiquement les chiffres de moindre importance. **Exécutez toujours ce script avant un cycle de release.**

## 5. Stratégie de logging
L'application utilise une classe `Logger` unifiée. Elle gère automatiquement la rotation des fichiers et les chemins selon le type de build :
*   **Mode Debug :** Les logs sont enregistrés dans `./logs/kub3_YYYY-MM-DD.log` (Local au dossier de build).
*   **Mode Release :** Les logs sont enregistrés dans `/var/log/kub3/kub3_YYYY-MM-DD.log` (Système).

## 6. Architecture & Logique
Le projet est strictement séparé en couches pour garantir la testabilité :
*   **`Kub3Core` (Bibliothèque statique) :** Contient le "Cerveau"—HAL (Hardware Abstraction Layer), FSM (Machine à états), et les Services. **Toute la logique métier doit résider ici.**
*   **`Kub3Software` (Exécutable) :** Contient le "Corps"—UI, ViewModels et le point d'entrée principal.
*   **`tests/` :** Contient les tests unitaires utilisant [Catch2](https://github.com/catchorg/Catch2).

## 7. Tests
Nous utilisons **Catch2** pour les tests de logique.
*   **Exécuter via Terminal :**
    ```bash
    cd build/<votre-dossier-preset>
    ctest --output-on-failure
    ```
*   **Exécuter via VS Code :** Utilisez l'onglet "Testing" (icône de bécher) dans la barre latérale.

## 8. FAQ et Dépannage
*   **"Undefined reference to vtable" :** Vous avez probablement ajouté une classe avec `Q_OBJECT` mais oublié d'ajouter le fichier d'en-tête (header) à la liste de `qt_add_library` ou `qt_add_executable` dans `CMakeLists.txt`. Vérifiez que tous les fichiers sont bien enregistrés.
*   **"MVS Lib not found" :** Assurez-vous que le SDK Hikrobot MVS est installé dans `/opt/MVS`. Si vous développez sur une machine sans caméra, le build émettra un avertissement mais compilera tout de même, avec les fonctionnalités de vision désactivées.
*   **Erreur d'édition de liens `cannot find -lKub3Core` :** Vous avez probablement renommé la bibliothèque. Assurez-vous que le nom de la cible dans `CMakeLists.txt` correspond exactement à ce que vous liez dans `target_link_libraries`.

***
*Pour toute question architecturale complémentaire, consultez la structure du répertoire `include/` qui correspond aux couches logiques de la machine à états KUB3.*
