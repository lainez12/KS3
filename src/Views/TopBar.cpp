#include <QHBoxLayout>
#include <QPixmap>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <Views/TopBar.h>
#include <Views/ViewBase.h>

namespace Kub3::UI::Views {

    TopBar::TopBar(QWidget *parent) : QFrame(parent) {
        setFrameShape(QFrame::NoFrame);
        setFrameShadow(QFrame::Raised);

        m_mainLayout = new QHBoxLayout(this);
        m_mainLayout->setContentsMargins(0, 0, 0, 0);
        m_mainLayout->setSpacing(0);

        m_logoKloe = new QLabel();
        m_logoKloe->setAlignment(Qt::AlignCenter);
        m_logoKloe->setPixmap(QPixmap(":/icons/logoKloeBlanc.svg"));
        m_mainLayout->addWidget(m_logoKloe);

        m_viewTitle = new QLabel();
        m_viewTitle->setAlignment(Qt::AlignCenter);
        m_viewTitle->setStyleSheet("font-size: 40px; font-weight: bold;");
        m_mainLayout->addStretch();
        m_mainLayout->addWidget(m_viewTitle);
        m_mainLayout->addStretch();

        m_sectionLayout = new QHBoxLayout();
        m_sectionLayout->setContentsMargins(0, 0, 0, 0);
        m_sectionLayout->setSpacing(10);

        m_sectionIcon = new QLabel();
        m_sectionIcon->setAlignment(Qt::AlignCenter);
        m_sectionLayout->addWidget(m_sectionIcon);

        m_sectionTitle = new QLabel();
        m_sectionTitle->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_sectionLayout->addWidget(m_sectionTitle);

        m_mainLayout->addLayout(m_sectionLayout);

        setLineWidth(1);
    }

    void TopBar::updateConfig(const TitleBarConfig &config) {
        m_currentConfig = const_cast<TitleBarConfig *>(&config);

        if (config.showTitleBar) {
            show();
            m_viewTitle->setText(config.viewTitle);
            m_logoKloe->setVisible(config.m_showLeftLogo);

            if (!config.iconPath.isEmpty()) {
                m_sectionIcon->setPixmap(QPixmap(config.iconPath));
                m_sectionIcon->setVisible(true);
                m_sectionTitle->setVisible(true);
            } else {
                m_sectionIcon->setVisible(false);
                m_sectionTitle->setVisible(false);
            }

            m_sectionTitle->setText(config.viewTitle);
            applyStylesheet(config.bgColor, config.textColor);
        } else {
            hide();
        }

        updateDimensions();
    }

    void TopBar::setVisible(bool visible) {
        if (visible && m_currentConfig && m_currentConfig->showTitleBar) {
            QFrame::show();
        } else if (!visible) {
            QFrame::hide();
        }
    }

    void TopBar::resizeEvent(QResizeEvent *event) {
        QFrame::resizeEvent(event);
        updateDimensions();
    }

    void TopBar::updateDimensions() {
        if (!parentWidget())
            return;

        int parentWidth  = parentWidget()->width();
        int parentHeight = parentWidget()->height();

        // Hauteur: 13% du parent
        int topBarHeight = static_cast<int>(parentHeight * 0.13);
        setFixedHeight(topBarHeight);

        // Padding horizontal: 2.5% de la largeur de TopBar
        int hPadding = static_cast<int>(parentWidth * 0.025);
        // Padding vertical: 21% de la hauteur de TopBar
        int vPadding = static_cast<int>(topBarHeight * 0.21);
        m_mainLayout->setContentsMargins(hPadding, vPadding, hPadding, vPadding);

        // Logo: 8% width, 38% height de TopBar
        int logoWidth  = static_cast<int>(parentWidth * 0.08);
        int logoHeight = static_cast<int>(topBarHeight * 0.38);
        m_logoKloe->setFixedSize(logoWidth, logoHeight);

        // Section: 17% width, 62% height de TopBar
        int sectionWidth  = static_cast<int>(parentWidth * 0.17);
        int sectionHeight = static_cast<int>(topBarHeight * 0.62);
        m_sectionIcon->setFixedSize(sectionHeight, sectionHeight);
    }

    void TopBar::applyStylesheet(const QColor &bgColor, const QColor &textColor) {
        QString bgStyle;
        if (bgColor.isValid()) {
            bgStyle = QString("background-color: %1;").arg(bgColor.name());
        }

        QString textStyle;
        if (textColor.isValid()) {
            textStyle = QString("color: %1;").arg(textColor.name());
        }

        setStyleSheet(bgStyle);
        m_viewTitle->setStyleSheet(textStyle);
        m_sectionTitle->setStyleSheet(textStyle);
    }

}
