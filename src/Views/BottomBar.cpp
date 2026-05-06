#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <Views/BottomBar.h>
#include <Views/Components/NavButton.h>

namespace Kub3::UI::Views {

    BottomBar::BottomBar(QWidget *parent) : QFrame(parent) {
        setFrameShape(QFrame::StyledPanel);
        setFrameShadow(QFrame::Raised);

        m_mainLayout = new QHBoxLayout(this);
        m_mainLayout->setContentsMargins(0, 0, 0, 0);
        m_mainLayout->setSpacing(0);

        m_leftLayout = new QHBoxLayout();
        m_leftLayout->setContentsMargins(0, 0, 0, 0);
        m_leftLayout->setSpacing(10);

        m_centerLayout = new QHBoxLayout();
        m_centerLayout->setContentsMargins(0, 0, 0, 0);
        m_centerLayout->setSpacing(10);

        m_rightLayout = new QHBoxLayout();
        m_rightLayout->setContentsMargins(0, 0, 0, 0);
        m_rightLayout->setSpacing(10);

        m_mainLayout->addLayout(m_leftLayout);
        m_mainLayout->addStretch();
        m_mainLayout->addLayout(m_centerLayout);
        m_mainLayout->addStretch();
        m_mainLayout->addLayout(m_rightLayout);

        // Shadow effect
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
        shadow->setBlurRadius(50);
        shadow->setOffset(0, 3);
        shadow->setColor(QColor(0, 112, 219, 160));
        setGraphicsEffect(shadow);
    }

    void BottomBar::updateButtonConfigs(const NavButtonManager &buttonManager, bool showCentralLogo) {
        clearButtons();

        if (showCentralLogo) {
            QLabel *logo = new QLabel();
            logo->setPixmap(QPixmap(":/icons/logoKloe.png").scaledToHeight(80));
            logo->setAlignment(Qt::AlignCenter);
            m_centerLayout->addWidget(logo);
        }

        for (const QString &position : QStringList{"left", "center", "right"}) {
            auto buttons = buttonManager.getButtonsByPosition(position);
            for (const auto &config : buttons) {
                if (!config.visible)
                    continue;

                NavButton *btn = createNavButton(config);
                if (!btn)
                    continue;

                NavButtonEntry entry;
                entry.button               = btn;
                entry.position             = position;
                m_buttons[config.buttonId] = entry;

                if (position == "left")
                    m_leftLayout->addWidget(btn);
                else if (position == "center")
                    m_centerLayout->addWidget(btn);
                else if (position == "right")
                    m_rightLayout->addWidget(btn);
            }
        }

        updateDimensions();
    }

    void BottomBar::updateButtonState(const QString &buttonId, bool newState) {
        auto it = m_buttons.find(buttonId);
        if (it != m_buttons.end() && it.value().button) {
            it.value().button->setEnabledNavButton(newState);
        }
    }

    void BottomBar::updateButtonText(const QString &buttonId, const QString &newText) {
        auto it = m_buttons.find(buttonId);
        if (it != m_buttons.end() && it.value().button) {
            it.value().button->setText(newText);
        }
    }

    void BottomBar::clearButtons() {
        auto clearLayout = [](QHBoxLayout *layout) {
            QLayoutItem *item;
            while ((item = layout->takeAt(0))) {
                if (item->widget())
                    item->widget()->deleteLater();
                delete item;
            }
        };

        clearLayout(m_leftLayout);
        clearLayout(m_centerLayout);
        clearLayout(m_rightLayout);

        m_buttons.clear();
    }

    NavButton *BottomBar::createNavButton(const NavButtonConfig &config) {
        NavButton *btn = new NavButton();
        btn->setup(config.text, config.color.name(), config.iconPath);
        btn->setEnabledNavButton(config.enabled);

        connect(btn, &NavButton::clicked, this, [this, config]() {
            if (config.callback) {
                config.callback(config.buttonId);
            }
            emit s_buttonClicked(config.buttonId);
        });

        return btn;
    }

    void BottomBar::resizeEvent(QResizeEvent *event) {
        QFrame::resizeEvent(event);
        updateDimensions();
    }

    void BottomBar::updateDimensions() {
        if (!parentWidget())
            return;

        int parentWidth  = parentWidget()->width();
        int parentHeight = parentWidget()->height();

        // Hauteur: 11% de la fenêtre principale
        int bottomBarHeight = static_cast<int>(parentHeight * 0.11);
        setFixedHeight(bottomBarHeight);

        // Padding horizontal: 2.5%, Vertical: 11.6%
        int hPadding = static_cast<int>(parentWidth * 0.025);
        int vPadding = static_cast<int>(bottomBarHeight * 0.116);
        m_mainLayout->setContentsMargins(hPadding, vPadding, hPadding, vPadding);
    }

}
