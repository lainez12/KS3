#include <Views/Components/Colors.h>
#include <Views/SettingsViewBase.h>

namespace Kub3::UI::Views
{

    SettingsViewBase::SettingsViewBase(Unique<ViewModels::BaseViewModel> viewModel, QWidget *parent) :
        ViewBase(std::move(viewModel), parent)
    {
        createNavButtonsConfigs();
    }

    void SettingsViewBase::createNavButtonsConfigs()
    {
        NavButtonConfig homeBtn(
            "Home",
            ":/icons/home.svg",
            "H",
            std::bind(&SettingsViewBase::onHomeButtonClicked, this, std::placeholders::_1));
        addNavButton("left", homeBtn);

        NavButtonConfig backBtn(
            "Back",
            ":/icons/back.svg",
            "B",
            std::bind(&SettingsViewBase::onBackButtonClicked, this, std::placeholders::_1));
        addNavButton("left", backBtn);
    }

    void SettingsViewBase::configTitleBar()
    {
        // setDefaultTitleBar("");
    }

    void SettingsViewBase::setDefaultTitleBar(const QString &viewTitle)
    {
        setTitleBar(TitleBarConfig{
            .viewTitle      = viewTitle,
            .textColor      = QColor("#FFF"),
            .bgColor        = QColor(BLUE_COLOR),
            .iconPath       = ":/icons/admin-circle.svg",
            .sectionTitle   = "Settings",
            .showTitleBar   = true,
            .m_showLeftLogo = true,
        });
    }

    void SettingsViewBase::onHomeButtonClicked(const QString &buttonId)
    {
        emit s_openView(Kub3::UI::ViewId::HOME_VIEW);
    }

    void SettingsViewBase::onBackButtonClicked(const QString &buttonId)
    {
        emit s_openView(Kub3::UI::ViewId::SETTINGS_VIEW);
    }

} // namespace Kub3::UI::Views