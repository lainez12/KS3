#include <QDebug>
#include <Views/Components/NavButtonManager.h>

namespace Kub3::UI::Views {

    void NavButtonManager::addButton(const QString &position, const NavButtonConfig &config, int order) {
        // Vérifier que la position est valide
        if (position != "left" && position != "center" && position != "right") {
            return;
        }

        // Retirer le bouton s'il existe déjà
        removeButton(config.buttonId);

        // Ajouter le bouton au stockage principal
        m_buttons.insert(config.buttonId, config);

        // Déterminer l'ordre automatiquement si non spécifié
        QVector<QString> &positionOrder = m_order[position];
        if (order < 0 || order > positionOrder.size()) {
            order = positionOrder.size();
        }

        // Insérer le buttonId à la position correcte
        positionOrder.insert(order, config.buttonId);
    }

    void NavButtonManager::removeButton(const QString &buttonId) {
        if (!m_buttons.contains(buttonId)) {
            return;
        }

        for (auto &positionOrder : m_order) {
            positionOrder.removeAll(buttonId);
        }

        m_buttons.remove(buttonId);
    }

    void NavButtonManager::clearButtons() {
        m_buttons.clear();
        m_order.clear();
    }

    QVector<NavButtonConfig> NavButtonManager::getButtonsByPosition(const QString &position) const {
        QVector<NavButtonConfig> result;

        // Récupérer l'ordre des IDs pour cette position
        if (!m_order.contains(position)) {
            return result;
        }

        const QVector<QString> &ids = m_order.value(position);

        // Ajouter les configurations dans l'ordre
        for (const QString &id : ids) {
            if (m_buttons.contains(id)) {
                result.append(m_buttons.value(id));
            }
        }

        return result;
    }

    NavButtonConfig *NavButtonManager::getButton(const QString &buttonId) {
        auto it = m_buttons.find(buttonId);
        return it != m_buttons.end() ? &it.value() : nullptr;
    }

    const NavButtonConfig *NavButtonManager::getButton(const QString &buttonId) const {
        auto it = m_buttons.find(buttonId);
        return it != m_buttons.end() ? &it.value() : nullptr;
    }

    bool NavButtonManager::setButtonEnabled(const QString &buttonId, bool enabled) {
        auto it = m_buttons.find(buttonId);
        if (it != m_buttons.end()) {
            it.value().enabled = enabled;
            return true;
        }
        return false;
    }

    bool NavButtonManager::setButtonVisible(const QString &buttonId, bool visible) {
        auto it = m_buttons.find(buttonId);
        if (it != m_buttons.end()) {
            it.value().visible = visible;
            return true;
        }
        return false;
    }

    bool NavButtonManager::setButtonText(const QString &buttonId, const QString &text) {
        auto it = m_buttons.find(buttonId);
        if (it != m_buttons.end()) {
            it.value().text = text;
            return true;
        }
        return false;
    }

    int NavButtonManager::buttonCountByPosition(const QString &position) const {
        if (!m_order.contains(position)) {
            return 0;
        }
        return m_order.value(position).size();
    }

} // namespace Kub3::UI::Views
