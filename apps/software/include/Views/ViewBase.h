#pragma once

#include <QColor>
#include <QHideEvent>
#include <QShowEvent>
#include <QString>
#include <QVector>
#include <QWidget>
#include <memory>

#include <ViewModels/BaseViewModel.h>
#include <Views/Components/MessageOverlay.h>
#include <Views/Components/NavButtonManager.h>
#include <Views/Components/PopUpMessage.h>
#include <utils.h>

#define UPDATE_DYNAMIC_PROPERTY(widget, propertyName, value) \
    do                                                       \
    {                                                        \
        (widget)->setProperty(propertyName, value);          \
        (widget)->style()->unpolish(widget);                 \
        (widget)->style()->polish(widget);                   \
        (widget)->update();                                  \
    } while (0)

namespace Kub3::UI
{
    enum class ViewId
    {
        EXPOSURE_MODE_VIEW,
        HOME_VIEW,
        EXPOSURE_MENU_VIEW,
        EXPOSURE_SETTINGS_VIEW,
        FAVORITE_EXPOSURE_SETTINGS_VIEW,
        COMPLETE_EXPOSURE_VIEW,
        PROGRESS_EXPOSURE_VIEW,
        RECAP_EXPOSURE_SETTINGS_VIEW,
        SAVE_EXPOSURE_SETTINGS_VIEW,
        SETTINGS_VIEW,
        SETTINGS_ABOUT_VIEW,
        SETTINGS_ADMIN_CONFIGURATOR_PASSWD_VIEW,
        SETTINGS_TEMPERATURE_VIEW,
        SETTINGS_OPERATING_TIMES_VIEW,
        SETTINGS_SCREENSHOT_EXPORT_VIEW,
        SETTINGS_LED_TEST_VIEW,
        SETTINGS_UPDATE_SOFTWARE_VIEW,
        ALIGNMENT_DISTANCE_VIEW,
        ALIGNMENT_CONTACT_SELECTION_VIEW,
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
        void resizeEventOverride(QResizeEvent *event);

        // --- Display Properties ---
        QString getViewTitle() const;
        bool showTitleBar() const;
        bool showCentralLogo() const;
        bool shadowedBoxStyle() const;
        const NavButtonManager &getNavButtonManager() const;
        void addNavButton(const QString &position, const NavButtonConfig &config, int order = -1);
        void removeNavButton(const QString &buttonId);
        void clearNavButtons();
        void setNavButtonEnabled(const QString &buttonId, bool enabled);
        void switchColorNavButton(const QString &buttonId, bool enabled);
        void setNavButtonVisible(const QString &buttonId, bool visible);
        void setNavButtonText(const QString &buttonId, const QString &text);
        void setTitleBar(const TitleBarConfig &titleBar);
        const TitleBarConfig &getTitleBar(void);

    signals:
        void s_openView(Kub3::UI::ViewId viewId);
        void s_goBack();
        void s_goBackHome();
        void s_buttonConfigsUpdated();

        void s_buttonStateChanged(const QString &buttonId, bool newState);
        void s_buttonTextChanged(const QString &buttonId, const QString &newText);
        void s_switchColorButton(const QString &buttonId, bool enabledColor);

    public slots:
        void ps_showOverlayMsg(QString text);
        void ps_createPopUp(const QString &title, const PopUpActions &buttons);
        void ps_createPopUpWithText(const QString &title, const PopUpActions &buttons, const QString &initialText);
        void ps_closePopUp();

    protected:
        void showEvent(QShowEvent *event) override;
        void hideEvent(QHideEvent *event) override;
        void setUpShadowedBoxStyle(QWidget *widget);

        template <typename T>
        T *getViewModel() const
        {
            return static_cast<T *>(m_viewModel.get());
        }

    protected:
        Unique<ViewModels::BaseViewModel> m_viewModel;
        bool m_showCentralLogo  = false;
        bool m_shadowedBoxStyle = true;
        NavButtonManager m_buttonManager;
        Unique<MessageOverlay> m_messages   = std::make_unique<MessageOverlay>(this);
        Unique<PopUpMessage> m_popUpMessage = std::make_unique<PopUpMessage>(this);

    private:
        virtual void createNavButtonsConfigs() = 0;
        virtual void configTitleBar()          = 0;

    private:
        TitleBarConfig m_titleBar = {};
    };

}