#pragma once

#include <QColor>
#include <QHideEvent>
#include <QShowEvent>
#include <QString>
#include <QWidget>
#include <memory>

#include <ViewModels/BaseViewModel.h>
#include <Views/Components/MessageOverlay.h>
#include <Views/Components/NavButtonManager.h>
#include <utils.h>

namespace Kub3::UI
{
    enum class ViewId
    {
        HOME_VIEW,
        HOME_EIGHT_VIEW,
        EXPOSURE_SETTINGS_VIEW,
        FAVORITE_EXPOSURE_SETTINGS_VIEW,
        COMPLETE_EXPOSURE_VIEW,
        PROGRESS_EXPOSURE_VIEW,
        RECAP_EXPOSURE_SETTINGS_VIEW,
        SAVE_EXPOSURE_SETTINGS_VIEW,
        SETTINGS_VIEW,
        SETTINGS_ABOUT_VIEW,
        SETTINGS_TEMPERATURE_VIEW,
        SETTINGS_OPERATING_TIMES_VIEW,
        SETTINGS_SCREENSHOT_EXPORT_VIEW,
        SETTINGS_LED_TEST_VIEW,
        SETTINGS_UPDATE_SOFTWARE_VIEW,
        ALIGNMENT_DISTANCE_VIEW,
        ALIGNMENT_VISUALISATION_VIEW,
        ALIGNMENT_LOAD_PARAMETERS_VIEW,
        ALIGNMENT_SAVE_PARAMETERS_VIEW,
        MACHINE_STATUS_VIEW
    };
}

namespace Kub3::UI::Views
{

    struct TitleBarConfig {
        QString viewTitle = "";
        QColor textColor;
        QColor bgColor;
        QString iconPath;
        QString sectionTitle;
        bool showTitleBar   = false;
        bool m_showLeftLogo = true;
    };

    class ViewBase : public QWidget
    {
        Q_OBJECT

    public:
        explicit ViewBase(Unique<ViewModels::BaseViewModel> viewModel, QWidget *parent = nullptr);
        virtual ~ViewBase() = default;

    public:
        void showAnOverlayMessage(QString text);

        void resizeEventOverride(QResizeEvent *event)
        {
            resizeEvent(event);
        }

        // --- Display Properties ---
        QString getViewTitle() const
        {
            return m_titleBar.viewTitle;
        }

        bool showTitleBar() const
        {
            return m_titleBar.showTitleBar;
        }

        bool showCentralLogo() const
        {
            return m_showCentralLogo;
        }

        bool shadowedBoxStyle() const
        {
            return m_shadowedBoxStyle;
        }

        const NavButtonManager &getNavButtonManager() const
        {
            return m_buttonManager;
        }

        void addNavButton(const QString &position, const NavButtonConfig &config, int order = -1)
        {
            m_buttonManager.addButton(position, config, order);
            emit s_buttonConfigsUpdated();
        }

        void removeNavButton(const QString &buttonId)
        {
            m_buttonManager.removeButton(buttonId);
            emit s_buttonConfigsUpdated();
        }

        void clearNavButtons()
        {
            m_buttonManager.clearButtons();
            emit s_buttonConfigsUpdated();
        }

        void setNavButtonEnabled(const QString &buttonId, bool enabled)
        {
            if (m_buttonManager.setButtonEnabled(buttonId, enabled))
            {
                emit s_buttonStateChanged(buttonId, enabled);
            }
        }

        void switchColorNavButton(const QString &buttonId, bool enabled)
        {
            NavButtonConfig *button = m_buttonManager.getButton(buttonId);
            if (button)
            {
                emit s_switchColorButton(buttonId, enabled);
            }
        }

        void setNavButtonVisible(const QString &buttonId, bool visible)
        {
            if (m_buttonManager.setButtonVisible(buttonId, visible))
            {
                emit s_buttonStateChanged(buttonId, visible);
            }
        }

        void setNavButtonText(const QString &buttonId, const QString &text)
        {
            if (m_buttonManager.setButtonText(buttonId, text))
            {
                emit s_buttonTextChanged(buttonId, text);
            }
        }

        void setTitleBar(const TitleBarConfig &titleBar)
        {
            m_titleBar = titleBar;
        }

        const TitleBarConfig &getTitleBar(void)
        {
            return m_titleBar;
        }

    signals:
        void s_openView(Kub3::UI::ViewId viewId);
        void s_goBack();
        void s_goBackHome();
        void s_buttonConfigsUpdated();

        void s_buttonStateChanged(const QString &buttonId, bool newState);
        void s_buttonTextChanged(const QString &buttonId, const QString &newText);
        void s_switchColorButton(const QString &buttonId, bool EnabledColor);

    protected:
        void showEvent(QShowEvent *event) override;
        void hideEvent(QHideEvent *event) override;
        void setUpShawedBoxStyle(QWidget *widget);

    protected:
        Unique<ViewModels::BaseViewModel> m_viewModel;
        bool m_showCentralLogo;
        bool m_shadowedBoxStyle = true;
        NavButtonManager m_buttonManager;
        Unique<MessageOverlay> m_messages = std::make_unique<MessageOverlay>(this);

    private:
        virtual void createNavButtonsConfigs() = 0;
        virtual void configTitleBar()          = 0;

    private:
        TitleBarConfig m_titleBar = {};
    };

}