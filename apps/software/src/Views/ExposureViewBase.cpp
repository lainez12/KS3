#include <Views/Components/Colors.h>
#include <Views/ExposureViewBase.h>

#define ID_BTN_VALIDATE "V"
#define ID_BTN_HOME     "H"

namespace Kub3::UI::Views
{

    ExposureViewBase::ExposureViewBase(Unique<ViewModels::BaseViewModel> viewModel, QWidget *parent) :
        ViewBase(std::move(viewModel), parent)
    {
    }

    void ExposureViewBase::createNavButtonsConfigs()
    {
        NavButtonConfig homeBtn(
            "Home",
            ":/icons/home.svg",
            "H",
            std::bind(&ExposureViewBase::onHomeButtonClicked, this, std::placeholders::_1));
        addNavButton("left", homeBtn, 0);

        NavButtonConfig validateBtn(
            "Validate",
            QColor(GREEN_COLOR),
            QColor(GREEN_COLOR_SHADOW),
            ":/icons/check.svg",
            ID_BTN_VALIDATE,
            std::bind(&ExposureViewBase::onValidateButtonClicked, this, std::placeholders::_1));
        addNavButton("right", validateBtn);

        setNavButtonEnabled(ID_BTN_VALIDATE, false);
    }

    void ExposureViewBase::configTitleBar()
    {
        // setDefaultTitleBar("");
    }

    void ExposureViewBase::setDefaultTitleBar(const QString &viewTitle)
    {
        setTitleBar(TitleBarConfig{
            .viewTitle      = viewTitle,
            .textColor      = QColor("#FFF"),
            .bgColor        = QColor(PURPLE_COLOR),
            .iconPath       = ":/icons/flood_icon.svg",
            .sectionTitle   = "Flood Exposure",
            .showTitleBar   = true,
            .m_showLeftLogo = true,
        });
    }

    void ExposureViewBase::onHomeButtonClicked(const QString &buttonId)
    {
        emit s_openView(Kub3::UI::ViewId::HOME_VIEW);
    }

} // namespace Kub3::UI::Views