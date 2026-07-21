#include <Views/AlignmentViewBase.h>
#include <Views/Components/Colors.h>

#define ID_BTN_VALIDATE "V"
#define ID_BTN_HOME     "H"
#define ID_BTN_BACK     "B"

namespace Kub3::UI::Views
{

    AlignmentViewBase::AlignmentViewBase(Unique<ViewModels::BaseViewModel> viewModel, QWidget *parent) :
        ViewBase(std::move(viewModel), parent)
    {
    }

    void AlignmentViewBase::createNavButtonsConfigs()
    {
        NavButtonConfig homeBtn(
            "Home",
            ":/icons/home.svg",
            "H",
            std::bind(&AlignmentViewBase::onHomeButtonClicked, this));
        addNavButton("left", homeBtn);

        NavButtonConfig backBtn(
            "Back",
            ":/icons/back.svg",
            "B",
            std::bind(&AlignmentViewBase::onBackButtonClicked, this));
        addNavButton("left", backBtn);

        NavButtonConfig validateBtn(
            "Validate",
            QColor(GREEN_COLOR),
            QColor(GREEN_COLOR_SHADOW),
            ":/icons/check.svg",
            ID_BTN_VALIDATE,
            std::bind(&AlignmentViewBase::onValidateButtonClicked, this));
        addNavButton("right", validateBtn);

        setNavButtonEnabled(ID_BTN_VALIDATE, false);
    }

    void AlignmentViewBase::configTitleBar()
    {
        // setDefaultTitleBar("");
    }

    void AlignmentViewBase::setDefaultTitleBar(const QString &viewTitle)
    {
        setTitleBar(TitleBarConfig{
            .viewTitle      = viewTitle,
            .textColor      = QColor("#FFF"),
            .bgColor        = QColor(TURQUOISE_COLOR),
            .iconPath       = ":/icons/mask-alignment_icon.svg",
            .sectionTitle   = "Mask Alignment",
            .showTitleBar   = true,
            .m_showLeftLogo = true,
        });
    }

    void AlignmentViewBase::onHomeButtonClicked()
    {
        emit s_openView(Kub3::UI::ViewId::EXPOSURE_MODE_VIEW);
    }

} // namespace Kub3::UI::Views