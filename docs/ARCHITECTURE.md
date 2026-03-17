### English Version

## Architecture & Compatibility (4", 6", and 8" Models)

The primary objective is to maintain a **single unified codebase** capable of driving the entire KUB3 product line.

*   **Common Core:** A shared software foundation used across all machines. It handles universal logic such as **Recipe Management** (exposure parameters), **Process Logging**, and global error handling.
*   **"Opinionated" Modularity:**
    *   Implementation of **Conditional Compilation**. During production, the build system generates an "optimized" executable locked to a specific model (e.g., a dedicated KUB3-6 binary). This eliminates the risk of incorrect model selection during on-site software updates.

## Required Software Stack (Technical Toolset)

To guarantee system performance and a 15+ year lifecycle:

*   **Language & Framework:**
    *   **C++20:** Provides raw performance and modern features (Concepts, Ranges) for a safer and more robust codebase compared to C++17.
    *   **Qt 6.8+ (LTS):** Used for the modern HMI, communication protocols (Serial, USB, Ethernet), and the **State Machine** framework.
*   **Industrial Vision:**
    *   **MVS Library (Hikrobot):** High-resolution camera control and micrometric alignment algorithms.
*   **Development Environment:**
    *   **CMake:** The global industry standard for build orchestration (replacing **qmake**, which is being deprecated).
    *   **IDE:** QtCreator, VS Code, or developer preference.
*   **Quality Analysis:**
    *   **Doxygen:** Automated generation of technical documentation from the source code.

## Methodologies (Rigor & Security)

*   **Version Control:**
    *   **Git:** Complete history tracking and the ability to "rollback" code in case of bugs (industry standard, replacing **Subversion**).
    *   **Hosting:** Usage of a secure Git server with issue tracking and CI/CD (e.g., Gitea or GitLab) to centralize code.
    *   **Backup:** Automated weekly full-server backups of the Git environment.
*   **Development Strategy:**
    *   **MVVM (Model-View-ViewModel):** Strict separation between UI design and calculation logic. This allows the interface to be modified without impacting machine control.
    *   **Finite State Machine (FSM):** Secures mechanical movements. A sequence (e.g., "UV Exposure") cannot start unless the safety conditions of the previous state (e.g., "Vacuum Engaged") are validated.
    *   **Hardware Abstraction Layer (HAL):** Decouples the core logic from the physical components. The HAL maps specific **motor types**, **sensors**, and **travel limits** to the machine's interfaces at compile-time, allowing the core program to remain agnostic of the specific model.
* **Testing & Deployment:**
   * **CI/CD (Continuous Integration / Deployment):** An automated pipeline triggered upon every push to the **deploy branch** that manages:
      * **Automated Versioning:** Automatic incrementation of `patch` or `test` numbers based on a `major.minor.patch.test` scheme (while major and minor increments remain manually managed).
      * **Automated Testing:** Execution of unit tests and, eventually, full system integration testing via a **KUB Simulator** designed to emulate hardware and MCU behavior. This ensures code integrity during software evolutions and guarantees a regression-free experience across all KUB3 models.
      * **Release Management:** Automatic generation of a new software release and changelog once all validation stages are successful.
---

### Version Française

## Architecture & Compatibilité (Modèles 4, 6 et 8 pouces)

L'objectif est d'avoir un seul code source pouvant être compilé pour piloter n'importe quelle machine de la gamme.

*   **Cœur Commun :** Une base logicielle identique pour toutes les machines. Elle gère la logique universelle telle que la **Gestion des Recettes** (paramètres d'insolation), l'**Historisation des Procédés** (logs) et la gestion globale des erreurs.
*   **Modularité "Opinionnée" :**
    *   Utilisation de la **compilation conditionnelle**. Lors d'une mise à jour logicielle en production, un exécutable "optimisé" est compilé et verrouillé pour chaque modèle (ex : un binaire dédié KUB3-6), évitant ainsi les erreurs de sélection de modèle.

## Liste des Outils Logiciels Requis (Stack Technique)

Pour garantir la performance et la modernité du système (maintenabilité sur 15 ans+) :

*   **Langage & Framework :**
    *   **C++20 :** Pour la performance brute et l'accès à une version moderne et sécurisée du langage (au lieu de C++17).
    *   **Qt 6.8+ (LTS) :** Pour l'interface graphique (IHM) moderne, la gestion des communications (Série, USB, Ethernet) et l'implémentation de la **State Machine**.
*   **Vision Industrielle :**
    *   **MVS Library (Hikrobot) :** Pilotage des caméras haute résolution et alignement micrométrique.
*   **Environnement de Développement :**
    *   **CMake :** L'outil standard mondial pour orchestrer la compilation (au lieu de **qmake** qui devient obsolète).
    *   **IDE :** QtCreator ou VS Code (selon préférence).
*   **Analyse de Qualité :**
    *   **Doxygen :** Pour générer automatiquement la documentation technique du code.

## Méthodologies (Rigueur et Sécurité)

*   **Gestion de versions (Versioning) :**
    *   **Git :** Historisation de chaque modification de code permettant de "revenir en arrière" en cas de bug (standard industriel, remplaçant **Subversion**).
    *   **Hébergement (Dépôt) :** Utilisation d'un serveur GIT sécurisé avec gestion de tickets et CI/CD (ex : Gitea ou GitLab) pour centraliser le code.
    *   **Backup :** Mise en place d'un script de sauvegarde hebdomadaire complète du serveur GIT.
*   **Stratégie de Développement :**
    *   **MVVM (Model-View-ViewModel) :** Séparation stricte entre l'affichage et la logique. On peut changer l'interface sans risquer de casser le pilotage de la machine.
    *   **Machine à États (FSM) :** Sécurisation des mouvements mécaniques. Une étape (ex : Exposition LED) ne peut démarrer que si l'état précédent (ex : Vide activé) est validé.
    *   **Couche d'Abstraction Matérielle (HAL) :** Découple la logique centrale des composants physiques. La HAL mappe les **types de moteurs**, les **capteurs** et les **limites de course** aux interfaces logiques lors de la compilation, permettant au cœur du programme de rester identique quel que soit le modèle.
* **Tests et Déploiement :**
   * **CI/CD (Intégration / Déploiement Continu) :** Un pipeline automatisé, déclenché à chaque push sur la branche **deploy**, assurant :
      * **Versionnage Automatique :** L'incrémentation automatique des numéros de `patch` ou de `test` selon le schéma `major.minor.patch.test` (les incréments majeurs et mineurs restant gérés manuellement).
      * **Tests Automatisés :** L'exécution des tests unitaires et, à terme, des tests d'intégration système via un **Simulateur KUB** conçu pour émuler le comportement matériel et des MCUs. Cela garantit l'intégrité du code lors des évolutions logicielles et l'absence de régression sur les différents modèles de KUB3.
      * **Gestion des Releases :** La génération automatique d'une nouvelle version (Release) et d'un journal des modifications (Changelog) une fois toutes les étapes de validation réussies.
---

## Architecture overview

```
 ===========================================================================
   LAYER 1: USER INTERFACE (VIEW & VIEWMODEL)
 ===========================================================================
 ┌────────────────────────────┐          ┌───────────────────────────────────────┐
 │      QML/QWidget VIEW      │  Binds   │            MAIN VIEWMODEL             │
 │      (Buttons, Charts)     │◄────────►│ (Exposes properties, handles clicks)  │
 └────────────────────────────┘          └────┬───────────────────────▲──────────┘
                                              │                       │
                                       Command│          Reads Live   │
                                       (Start)│          Data (Polls) │
                                              ▼                       │
 ===========================================================================
   LAYER 2: THE BRAIN (ORCHESTRATION)
 ===========================================================================
 ┌─────────────────────────────────────────────────────────────────────────┐
 │                            MASTER FSM                                   │
 │ ----------------------------------------------------------------------- │
 │ - State: IDLE -> LOADING -> ALIGNING -> EXPOSING                        │
 │ - Role: Doesn't know *how* to load, just *when* to load.                │
 └───────┬──────────────────────┬───────────────────────┬──────────────────┘
         │ Triggers             │ Triggers              │ Triggers
         ▼                      ▼                       ▼
 ┌───────────────┐      ┌───────────────┐       ┌───────────────┐
 │ WAFER LOADER  │      │   ALIGNMENT   │       │   EXPOSURE    │
 │  CONTROLLER   │      │   CONTROLLER  │       │   CONTROLLER  │
 │ (Micro-Logic) │      │ (Micro-Logic) │       │ (Micro-Logic) │
 └───────┬───────┘      └───────┬───────┘       └───────┬───────┘
         │                      │                       │
         │ logic: "If Vac ON,   │ logic: "Calc offset,  │ logic: "Safe?"
         │ move Z, then X"      │ move X, Y, Theta"     │ Open Shutter"
         │                      │                       │
         └──────────┬───────────┴───────────┬───────────┘
                    │ Uses Shared Resources │
                    ▼                       ▼
 ===========================================================================
   LAYER 3: HARDWARE SERVICES (CAPABILITIES) & DATA REPOSITORY
 ===========================================================================
 ┌───────────────────────────┐      ┌──────────────────────────────────────┐
 │  SHARED HARDWARE SERVICES │      │      MACHINE STATUS REPOSITORY       │
 │      (The "Hands")        │      │      (The "Single Source of Truth")  │
 │ ------------------------- │      │ ------------------------------------ │
 │ 1. UnifiedMotorService    │      │ - struct MachineData {               │
 │ 2. IOService (Vac/Valves) │      │     atomic<double> posX, posY;       │
 │ 3. CameraService          │      │     atomic<bool> vacuumState;        │
 │ 4. UVService              │      │     atomic<bool> drawerClosed;       │
 └─────────────┬─────────────┘      │     atomic<float> uvIntensity;       │
               │                    │   }                                  │
               │                    └────────────────▲─────────────────────┘
               │ Sends Commands                      │
               ▼                                     │ Writes Updates
 ===========================================================================
   LAYER 4: DRIVERS & PHYSICAL LINK (INFRASTRUCTURE)
 ===========================================================================
 ┌───────────────────────────┐                       │
 │    MCU / DEVICE DRIVERS   │───────────────────────┘
 │ (Protocol Parsers, Threads)
 │ ------------------------- │
 │ - McuDriver_Main (XYT)    │
 │ - McuDriver_Aux (Z/Load)  │
 │ - CameraDriver (SDK)      │
 └─────────────┬─────────────┘
               │ RS232 / USB / Ethernet
       ┌───────▼───────┐
       │ PHYSICAL HW   │
       └───────────────┘
```
