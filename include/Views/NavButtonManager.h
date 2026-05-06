#pragma once

#include <QColor>
#include <QMap>
#include <QString>
#include <QVector>
#include <functional>

namespace Kub3::UI::Views {
    /**
     * @struct NavButtonConfig
     * Configuration pour un bouton de navigation
     *
     * Chaque bouton a :
     * - Une position : "left", "center", "right"
     * - Un ordre : index dans la position
     * - Des propriétés d'affichage et de comportement
     */
    struct NavButtonConfig {
        QString text;
        QColor color;
        QString iconPath;
        QString buttonId;
        std::function<void(const QString &)> callback;
        bool enabled = true;
        bool visible = true;

        NavButtonConfig() = default;

        NavButtonConfig(const QString &t, const QColor &c, const QString &i, const QString &id, std::function<void(const QString &)> cb) : text(t), color(c), iconPath(i), buttonId(id), callback(cb) {
        }
    };

    /**
     * @class NavButtonManager
     * Gestionnaire centralisé pour les boutons de navigation
     *
     * Architecture simple et efficace :
     * - m_buttons: QMap pour accès O(log n) par buttonId
     * - m_order: Suivi de l'ordre des boutons par position
     *
     * Responsabilités :
     * - Stocker les configurations des boutons avec leurs positions
     * - Permettre l'ajout/suppression/modification de boutons
     * - Gérer l'ordre des boutons dans chaque position
     * - Fournir des listes groupées par position
     */
    class NavButtonManager {
    public:
        NavButtonManager()          = default;
        virtual ~NavButtonManager() = default;

        /**
         * Ajoute ou met à jour un bouton à une position donnée
         * @param position Position du bouton ("left", "center", "right")
         * @param config Configuration du bouton
         * @param order Ordre d'apparition dans la position (optionnel, auto-incrémenté)
         */
        void addButton(const QString &position, const NavButtonConfig &config, int order = -1);

        /**
         * Retire un bouton par son ID
         */
        void removeButton(const QString &buttonId);

        /**
         * Retire tous les boutons
         */
        void clearButtons();

        /**
         * Retourne les boutons d'une position spécifique, dans l'ordre
         */
        QVector<NavButtonConfig> getButtonsByPosition(const QString &position) const;

        /**
         * Retourne la configuration d'un bouton par son ID
         */
        NavButtonConfig *getButton(const QString &buttonId);
        const NavButtonConfig *getButton(const QString &buttonId) const;

        /**
         * Change l'état activé/désactivé d'un bouton
         */
        bool setButtonEnabled(const QString &buttonId, bool enabled);

        /**
         * Change la visibilité d'un bouton
         */
        bool setButtonVisible(const QString &buttonId, bool visible);

        /**
         * Change le texte d'un bouton
         */
        bool setButtonText(const QString &buttonId, const QString &text);

        /**
         * Retourne le nombre total de boutons
         */
        int buttonCount() const {
            return m_buttons.size();
        }

        /**
         * Retourne le nombre de boutons pour une position donnée
         */
        int buttonCountByPosition(const QString &position) const;

        /**
         * Retourne si le gestionnaire est vide
         */
        bool isEmpty() const {
            return m_buttons.isEmpty();
        }

    private:
        // Stockage principal: buttonId → config (O(log n) lookup)
        QMap<QString, NavButtonConfig> m_buttons;

        // Suivi de l'ordre par position: position → [id1, id2, id3...]
        QMap<QString, QVector<QString>> m_order;
    };

} // namespace Kub3::UI::Views
