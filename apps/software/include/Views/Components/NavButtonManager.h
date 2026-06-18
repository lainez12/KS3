#pragma once

#include <QColor>
#include <QMap>
#include <QString>
#include <QVector>
#include <Views/Components/Colors.h>
#include <functional>

namespace Kub3::UI::Views {

    struct NavButtonConfig {
        QString text;
        QColor colorEnabled  = QColor(BLUE_COLOR);
        QColor colorDisabled = QColor(BLUE_COLOR_SHADOW);
        QString iconPath;
        QString buttonId;
        std::function<void(const QString &)> callback;
        bool enabled = true;
        bool visible = true;

        NavButtonConfig() = default;

        NavButtonConfig(const QString &t, const QColor &cE, const QColor &cD, const QString &i, const QString &id, std::function<void(const QString &)> cb) : text(t), colorEnabled(cE), colorDisabled(cD), iconPath(i), buttonId(id), callback(cb) {
        }

        NavButtonConfig(const QString &t, const QString &i, const QString &id, std::function<void(const QString &)> cb) : text(t), iconPath(i), buttonId(id), callback(cb) {
        }
    };

    class NavButtonManager {
    public:
        NavButtonManager()          = default;
        virtual ~NavButtonManager() = default;

        void addButton(const QString &position, const NavButtonConfig &config, int order = -1);

        void removeButton(const QString &buttonId);

        void clearButtons();

        QVector<NavButtonConfig> getButtonsByPosition(const QString &position) const;

        NavButtonConfig *getButton(const QString &buttonId);
        const NavButtonConfig *getButton(const QString &buttonId) const;

        bool setButtonEnabled(const QString &buttonId, bool enabled);

        bool setButtonVisible(const QString &buttonId, bool visible);

        bool setButtonText(const QString &buttonId, const QString &text);

        int buttonCount() const {
            return m_buttons.size();
        }

        int buttonCountByPosition(const QString &position) const;

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
