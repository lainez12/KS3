#include <Views/AlignmentViewBase.h>
#include <Views/Components/Colors.h>

#define ID_BTN_VALIDATE "V"
#define ID_BTN_HOME     "H"
#define ID_BTN_BACK     "B"

namespace Kub3::UI::Views {

    AlignmentViewBase::AlignmentViewBase(
        Unique<ViewModels::IViewModel> viewModel,
        QWidget *parent) :
        ViewBase(std::move(viewModel), parent) {
    }

    void AlignmentViewBase::createNavButtonsConfigs() {
        NavButtonConfig homeBtn(
            "Home",
            ":/icons/home.svg",
            "H",
            std::bind(&AlignmentViewBase::onHomeButtonClicked, this, std::placeholders::_1));
        addNavButton("left", homeBtn);

        NavButtonConfig backBtn(
            "Back",
            ":/icons/back.svg",
            "B",
            std::bind(&AlignmentViewBase::onBackButtonClicked, this, std::placeholders::_1));
        addNavButton("left", backBtn);

        NavButtonConfig validateBtn(
            "Validate",
            QColor(GREEN_COLOR),
            QColor(GREEN_COLOR_SHADOW),
            ":/icons/check.svg",
            ID_BTN_VALIDATE,
            std::bind(&AlignmentViewBase::onValidateButtonClicked, this, std::placeholders::_1));
        addNavButton("right", validateBtn);

        setNavButtonEnabled(ID_BTN_VALIDATE, false);
    }

    void AlignmentViewBase::configTitleBar() {
        // setDefaultTitleBar("");
    }

    void AlignmentViewBase::setDefaultTitleBar(const QString &viewTitle) {
        m_titleBar = TitleBarConfig(
            viewTitle,
            QColor("#FFF"),
            QColor(TURQUOISE_COLOR),
            ":/icons/flood_icon.svg",
            "Mask Alignment",
            true,
            true);
    }

    void AlignmentViewBase::onHomeButtonClicked(const QString &buttonId) {
        emit s_openView(Kub3::UI::ViewId::HOME_VIEW);
    }

} // namespace Kub3::UI::Views