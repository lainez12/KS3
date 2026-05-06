#pragma once

#include <QColor>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QString>

namespace Kub3::UI::Views {

    struct TitleBarConfig;

    class TopBar : public QFrame {
        Q_OBJECT

    public:
        explicit TopBar(QWidget *parent = nullptr);
        ~TopBar() = default;

        void updateConfig(const TitleBarConfig &config);
        void setVisible(bool visible);

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        void updateLayout();
        void updateDimensions();
        void applyStylesheet(const QColor &bgColor, const QColor &textColor);

        QLabel *m_logoKloe           = nullptr;
        QLabel *m_viewTitle          = nullptr;
        QLabel *m_sectionIcon        = nullptr;
        QLabel *m_sectionTitle       = nullptr;
        QHBoxLayout *m_mainLayout    = nullptr;
        QHBoxLayout *m_sectionLayout = nullptr;

        int m_parentWidth               = 0;
        int m_parentHeight              = 0;
        TitleBarConfig *m_currentConfig = nullptr;
    };

}
