#pragma once

#include <QFrame>
#include <QHBoxLayout>
#include <QMap>
#include <QPointer>
#include <Views/NavButtonManager.h>
#include <Views/Components/NavButton.h>

namespace Kub3::UI::Views {

    class BottomBar : public QFrame {
        Q_OBJECT

    public:
        explicit BottomBar(QWidget *parent = nullptr);
        ~BottomBar() = default;

        void updateButtonConfigs(const NavButtonManager &buttonManager, bool showCentralLogo);
        void updateButtonState(const QString &buttonId, bool newState);
        void updateButtonText(const QString &buttonId, const QString &newText);
        void clearButtons();

    signals:
        void s_buttonClicked(const QString &buttonId);

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        void updateDimensions();
        NavButton *createNavButton(const NavButtonConfig &config);

        struct NavButtonEntry {
            QPointer<NavButton> button = nullptr;
            QString position;
        };

        QHBoxLayout *m_mainLayout   = nullptr;
        QHBoxLayout *m_leftLayout   = nullptr;
        QHBoxLayout *m_centerLayout = nullptr;
        QHBoxLayout *m_rightLayout  = nullptr;
        QMap<QString, NavButtonEntry> m_buttons;

        int m_parentWidth  = 0;
        int m_parentHeight = 0;
    };

}
