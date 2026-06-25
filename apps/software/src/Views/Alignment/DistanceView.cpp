#include <QString>

#include "ui_DistanceView.h"
#include <Views/Alignment/DistanceView.h>
#include <Views/Components/Colors.h>

#define ID_BTN_BACK     "B"
#define ID_BTN_SAVE     "S"
#define ID_BTN_HOME     "H"
#define ID_BTN_VALIDATE "V"

DistanceView::DistanceView(Unique<DistanceViewModel> viewModel, QWidget *parent) :
    AlignmentViewBase(std::move(viewModel), parent),
    m_keyboard(this),
    ui(new Ui::DistanceView) {
    ui->setupUi(this);
    m_keyboard.setupKeyboardConnections(this);
    setDefaultTitleBar("Mask-to-substrate distance");
    createNavButtonsConfigs();
    setNavButtonEnabled(ID_BTN_VALIDATE, true);
}
DistanceView::~DistanceView() {
}


void DistanceView::setNewNavButtonsConfigs() {
}

void DistanceView::onBackButtonClicked(const QString &buttonId) {
}

void DistanceView::onValidateButtonClicked(const QString &buttonId) {
    emit s_openView(Kub3::UI::ViewId::ALIGNMENT_VISUALISATION_VIEW);
}