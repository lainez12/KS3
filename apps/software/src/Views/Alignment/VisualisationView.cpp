#include <QString>

#include <Views/Alignment/VisualisationView.h>
#include <Views/Components/Colors.h>
#include <Views/Components/RealPositionCameras.h>

#include "ui_VisualisationView.h"

#define ORANGE_BTN_SIZE 90

#define ID_BTN_BACK     "B"
#define ID_BTN_SAVE     "S"
#define ID_BTN_HOME     "H"
#define ID_BTN_VALIDATE "V"

VisualisationView::VisualisationView(Unique<VisualisationViewModel> viewModel, QWidget *parent) :
    AlignmentViewBase(std::move(viewModel), parent),
    PadReceiverViewTrait(this),
    m_keyboard(this),
    ui(new Ui::VisualisationView)
{
    setupUI();
    setupConnections();
    setupBindings();
    setupNavButtons();
}

VisualisationView::~VisualisationView()
{
}

void VisualisationView::setupUI()
{
    ui->setupUi(this);

    // Reparent floating components to ensure they overlap properly
    ui->camAndMaskDistContainer->setParent(ui->content);
    ui->leftCamCtrlBtnsContainer->setParent(ui->content);
    ui->rightCamCtrlBtnsContainer->setParent(ui->content);

    // Z-Order assignments
    ui->visionContainer->raise();           // Layer 1: Background video
    ui->camAndMaskDistContainer->raise();   // Layer 2: Center indicators
    ui->moveLeftCamWidget->raise();         // Layer 3: Left slide-out panel
    ui->moveRightCamWidget->raise();        // Layer 3: Right slide-out panel
    ui->leftCamCtrlBtnsContainer->raise();  // Layer 4: Left orange buttons
    ui->rightCamCtrlBtnsContainer->raise(); // Layer 4: Right orange buttons

    setDefaultTitleBar("Visualisation");
    m_shadowedBoxStyle = false;

    // Sub-widget sizing configurations
    QList<NavButton *> navBtns = {
        ui->moveCamLeft, ui->speedCamLeft, ui->gotoLeft, ui->pickUpLeft,
        ui->gotoRight, ui->speedCamRight, ui->moveCamRight, ui->pickUpRight};

    ui->leftCamCtrlBtnsContainer->setFixedHeight(ORANGE_BTN_SIZE + 20);
    ui->rightCamCtrlBtnsContainer->setFixedHeight(ORANGE_BTN_SIZE + 20);
    for (auto btn : navBtns)
    {
        btn->setFixedSize(ORANGE_BTN_SIZE, ORANGE_BTN_SIZE);
    }

    // Setup Action Buttons
    ui->speedCamRight->setup("Cam. Speed", QColor(ORANGE_COLOR), QColor(TURQUOISE_COLOR), ":/icons/speed-motor-low.svg", QFont("Arial", 12), 8, "#fff");
    ui->speedCamLeft->setup("Cam. Speed", QColor(ORANGE_COLOR), QColor(TURQUOISE_COLOR), ":/icons/speed-motor-low.svg", QFont("Arial", 12), 8, "#fff");
    ui->pickUpRight->setup("Go to X/Y", QColor(ORANGE_COLOR), QColor(TURQUOISE_COLOR), ":/icons/go-to.svg", QFont("Arial", 12), 8, "#fff");
    ui->pickUpLeft->setup("Go to X/Y", QColor(ORANGE_COLOR), QColor(TURQUOISE_COLOR), ":/icons/go-to.svg", QFont("Arial", 12), 8, "#fff");
    ui->gotoRight->setup("Pick Up X/Y", QColor(ORANGE_COLOR), QColor(TURQUOISE_COLOR), ":/icons/pick-up-xy.svg", QFont("Arial", 12), 8, "#fff");
    ui->gotoLeft->setup("Pick Up X/Y", QColor(ORANGE_COLOR), QColor(TURQUOISE_COLOR), ":/icons/pick-up-xy.svg", QFont("Arial", 12), 8, "#fff");
    ui->moveCamRight->setup("Move. Cam", QColor(ORANGE_COLOR), QColor(TURQUOISE_COLOR), ":/icons/move-cam.svg", QFont("Arial", 12), 8, "#fff");
    ui->moveCamLeft->setup("Move. Cam", QColor(ORANGE_COLOR), QColor(TURQUOISE_COLOR), ":/icons/move-cam.svg", QFont("Arial", 12), 8, "#fff");

    ui->configCamLeftFrame->setVisible(false);
    ui->configCamRightFrame->setVisible(false);
    ui->moveLeftCamWidget->setVisible(false);
    ui->moveRightCamWidget->setVisible(false);
    ui->visualMarkLeft->setVisible(false);
    ui->visualMarkRight->setVisible(false);

    // Dynamic overlay widgets creation
    m_mapPositionCameras = new RealPositionCameras(this);
    m_mapPositionCameras->setMinimumSize(0, 64);
    m_mapPositionCameras->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    // Inject position map at the TOP (index 0) of the container layout
    ui->camAndMaskDistContainerLayout->insertWidget(0, m_mapPositionCameras);

    // Hard Force Contact Form setup
    m_hardForceContactForm = new HardForceContactForm(this);
    m_hardForceContactForm->setVisible(false);
}

void VisualisationView::setupConnections()
{
    m_keyboard.setupKeyboardConnections(this, "_d");
    m_keyboard.setupKeyboardConnections(this, "_g");

    // Camera / Vision pipeline updates
    if (auto *vm = dynamic_cast<VisualisationViewModel *>(m_viewModel.get()))
    {
        connect(
            vm,
            &Kub3::UI::ViewModels::BaseVisionViewModel::s_frameReady,
            this,
            [this](const QString &cameraId, const QImage &frame) {
                if (cameraId == UPPER_LEFT_CAMERA)
                    ui->visioLeft->ps_onFrameUpdated(frame);
                else if (cameraId == UPPER_RIGHT_CAMERA)
                    ui->visioRight->ps_onFrameUpdated(frame);
            });
    }

    // Toggle event connections
    connect(ui->configCamLeftCheck, &QCheckBox::toggled, this, &VisualisationView::leftCamConfigToggled);
    connect(ui->configCamRightCheck, &QCheckBox::toggled, this, &VisualisationView::rightCamConfigToggled);

    connect(ui->moveCamLeft, &NavButton::clicked, [this]() { navButtonToggled(ui->moveCamLeft, ui->moveLeftCamWidget); });
    connect(ui->moveCamRight, &NavButton::clicked, [this]() { navButtonToggled(ui->moveCamRight, ui->moveRightCamWidget); });

    // Real Position Map layout updates
    connect(m_mapPositionCameras, &RealPositionCameras::s_openMap, this, &VisualisationView::mapPositionCamerasOpenMap);
    connect(m_mapPositionCameras, &RealPositionCameras::s_closeMap, this, &VisualisationView::mapPositionCamerasCloseMap);
}

void VisualisationView::setupBindings()
{
    // Local state translation helper
    auto toMovementKind = [](UI::Views::PadTrigger trigger) -> MovementKind {
        switch (trigger)
        {
        case UI::Views::PadTrigger::Pressed:
            return MovementKind::GRANULAR;
        case UI::Views::PadTrigger::Held:
            return MovementKind::CONTINUOUS;
        case UI::Views::PadTrigger::Released:
            return MovementKind::STOP;
        }
        return MovementKind::STOP;
    };
    // Camera action-binding generator
    auto bindCamera = [this, toMovementKind](CameraId camId, CameraDirection dir) {
        return [this, toMovementKind, camId, dir](UI::Views::PadTrigger trigger) {
            cameraMovement(camId, toMovementKind(trigger), dir);
        };
    };
    // Stage action-binding generator
    auto bindStage = [this, toMovementKind](AlignmentStageId stageId, AlignmentStageDirection dir) {
        return [this, toMovementKind, stageId, dir](UI::Views::PadTrigger trigger) {
            alignmentStageMovement(stageId, toMovementKind(trigger), dir);
        };
    };

    // Logical Pad Mapping to Cameras
    // --- Left camera
    link(UI::Views::PadTarget::LeftCamera, UI::Views::PadAction::Up, bindCamera(CameraId::LEFT, CameraDirection::UP));
    link(UI::Views::PadTarget::LeftCamera, UI::Views::PadAction::Left, bindCamera(CameraId::LEFT, CameraDirection::LEFT));
    link(UI::Views::PadTarget::LeftCamera, UI::Views::PadAction::Down, bindCamera(CameraId::LEFT, CameraDirection::DOWN));
    link(UI::Views::PadTarget::LeftCamera, UI::Views::PadAction::Right, bindCamera(CameraId::LEFT, CameraDirection::RIGHT));
    // --- Right camera
    link(UI::Views::PadTarget::RightCamera, UI::Views::PadAction::Up, bindCamera(CameraId::RIGHT, CameraDirection::UP));
    link(UI::Views::PadTarget::RightCamera, UI::Views::PadAction::Left, bindCamera(CameraId::RIGHT, CameraDirection::LEFT));
    link(UI::Views::PadTarget::RightCamera, UI::Views::PadAction::Down, bindCamera(CameraId::RIGHT, CameraDirection::DOWN));
    link(UI::Views::PadTarget::RightCamera, UI::Views::PadAction::Right, bindCamera(CameraId::RIGHT, CameraDirection::RIGHT));

    // Logical Pad Mapping to Alignment Stages
    // --- X stage
    link(UI::Views::PadTarget::XStage, UI::Views::PadAction::Left, bindStage(AlignmentStageId::X, AlignmentStageDirection::X_LEFT));
    link(UI::Views::PadTarget::XStage, UI::Views::PadAction::Right, bindStage(AlignmentStageId::X, AlignmentStageDirection::X_RIGHT));
    // --- Y stage
    link(UI::Views::PadTarget::YStage, UI::Views::PadAction::Front, bindStage(AlignmentStageId::Y, AlignmentStageDirection::Y_FRONT));
    link(UI::Views::PadTarget::YStage, UI::Views::PadAction::Back, bindStage(AlignmentStageId::Y, AlignmentStageDirection::Y_BACK));
    // --- Theta stage
    link(UI::Views::PadTarget::ThetaStage, UI::Views::PadAction::CW, bindStage(AlignmentStageId::THETA, AlignmentStageDirection::THETA_CW));
    link(UI::Views::PadTarget::ThetaStage, UI::Views::PadAction::CCW, bindStage(AlignmentStageId::THETA, AlignmentStageDirection::THETA_CCW));
}

void VisualisationView::setupNavButtons()
{
    createNavButtonsConfigs();
    setNavButtonEnabled(ID_BTN_VALIDATE, true);
    setNewNavButtonsConfigs();
}

void VisualisationView::cameraMovement(CameraId camId, MovementKind kind, CameraDirection dir)
{
    if (auto *vm = dynamic_cast<VisualisationViewModel *>(m_viewModel.get()))
    {
        vm->uiRequestCameraMovement(camId, kind, dir);
    }
}

void VisualisationView::alignmentStageMovement(AlignmentStageId stageId, MovementKind kind, AlignmentStageDirection dir)
{
    if (auto *vm = dynamic_cast<VisualisationViewModel *>(m_viewModel.get()))
    {
        vm->uiRequestAlignmentStageMovement(stageId, kind, dir);
    }
}

void VisualisationView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
    updateOverlayPositions();
}

void VisualisationView::updateOverlayPositions()
{
    int w = ui->content->width();
    int h = ui->content->height();

    // 1. Vision Container (Full Background)
    ui->visionContainer->setGeometry(0, 0, w, h);

    // 2. Slide-out Coordinate Panels
    int leftWidth = ui->moveLeftCamWidget->width();
    ui->moveLeftCamWidget->setGeometry(0, 0, leftWidth, h);

    int rightWidth = ui->moveRightCamWidget->width();
    ui->moveRightCamWidget->setGeometry(w - rightWidth, 0, rightWidth, h);

    // 3. Center Bottom Group (Real-time pos & Masking distance)
    int centralW = ui->camAndMaskDistContainer->width();
    int centralH = ui->camAndMaskDistContainer->height();
    ui->camAndMaskDistContainer->setGeometry((w - centralW) / 2, h - centralH, centralW, centralH);

    // 4. Static Orange Buttons
    constexpr int PADDING = 20;

    int leftBtnsH = ui->leftCamCtrlBtnsContainer->height();
    ui->leftCamCtrlBtnsContainer->setGeometry(0, h - leftBtnsH, (w - centralW) / 2, leftBtnsH);

    int targetRightWidth = (w - centralW) / 2;
    int rightBtnsH       = ui->rightCamCtrlBtnsContainer->height();
    ui->rightCamCtrlBtnsContainer->setGeometry(w - targetRightWidth, h - rightBtnsH, targetRightWidth, rightBtnsH);

    // Absolute position the Hard Force Contact Form to bottom center as well
    m_hardForceContactForm->setGeometry(
        (w - m_hardForceContactForm->width()) / 2,
        h - m_hardForceContactForm->height() + (ui->camAndMaskDistContainer->height() / 4),
        centralW,
        m_hardForceContactForm->height());
}

// ==========================================
// COMPONENT SLOTS & HANDLERS
// ==========================================

void VisualisationView::mapPositionCamerasOpenMap()
{
    ui->camAndMaskDistContainer->adjustSize(); // Updates container height
    updateOverlayPositions();                  // Re-anchors upwards

    ui->configCamRightCheck->setChecked(false);
    ui->configCamLeftCheck->setChecked(false);
}

void VisualisationView::mapPositionCamerasCloseMap()
{
    ui->camAndMaskDistContainer->adjustSize(); // Shrinks container height
    updateOverlayPositions();                  // Re-anchors downwards
}

void VisualisationView::setNewNavButtonsConfigs()
{
    NavButtonConfig save("Save", ":/icons/save.svg", "S", std::bind(&VisualisationView::onSaveButtonClicked, this, std::placeholders::_1));
    addNavButton("center", save);

    NavButtonConfig load("Load", ":/icons/load.svg", "L", std::bind(&VisualisationView::onLoadButtonClicked, this, std::placeholders::_1));
    addNavButton("center", load);

    NavButtonConfig screenshot("Screenshot", ":/icons/screenshot.svg", "P", std::bind(&VisualisationView::onScreenshotButtonClicked, this, std::placeholders::_1));
    addNavButton("center", screenshot);

    NavButtonConfig hardForceCont("Hard Force Cont.", QColor(BLUE_COLOR), QColor(TURQUOISE_COLOR), ":/icons/hard-force-contact.svg", "F", std::bind(&VisualisationView::onHardForceContButtonClicked, this, std::placeholders::_1));
    addNavButton("center", hardForceCont);

    NavButtonConfig speedMotorSubst("Subst. Speed", QColor(BLUE_COLOR), QColor(TURQUOISE_COLOR), ":/icons/speed-motor-subst.svg", "U", std::bind(&VisualisationView::onSpeedMotorSubstButtonClicked, this, std::placeholders::_1));
    addNavButton("center", speedMotorSubst);

    NavButtonConfig switchVacumAir("Vacum - Air.", QColor(BLUE_COLOR), QColor(TURQUOISE_COLOR), ":/icons/vac_air_switch.svg", "M", std::bind(&VisualisationView::onSwitchVacumAirButtonClicked, this, std::placeholders::_1));
    addNavButton("center", switchVacumAir);

    NavButtonConfig visualMark("Visual Mark", QColor(BLUE_COLOR), QColor(TURQUOISE_COLOR), ":/icons/visual-mark.svg", "I", std::bind(&VisualisationView::onVisualMarkButtonClicked, this, std::placeholders::_1));
    addNavButton("center", visualMark);
}

void VisualisationView::leftCamConfigToggled(bool checked)
{
    ui->configCamLeftFrame->setVisible(checked);
    if (checked)
    {
        ui->configCamLeftCheck->setIcon(QIcon(":/icons/cam-settings-right-checked.svg"));
        m_mapPositionCameras->closeMap();
        closeHardForceContactFormIfNeeded();
    }
    else
    {
        ui->configCamLeftCheck->setIcon(QIcon(":/icons/cam-settings-left.svg"));
    }
}

void VisualisationView::rightCamConfigToggled(bool checked)
{
    ui->configCamRightFrame->setVisible(checked);
    if (checked)
    {
        ui->configCamRightCheck->setIcon(QIcon(":/icons/cam-settings-left-checked.svg"));
        m_mapPositionCameras->closeMap();
        closeHardForceContactFormIfNeeded();
    }
    else
    {
        ui->configCamRightCheck->setIcon(QIcon(":/icons/cam-settings-right.svg"));
    }
}

// ==========================================
// NAV BUTTON CALLBACKS
// ==========================================

void VisualisationView::onBackButtonClicked(const QString &buttonId) {}

void VisualisationView::onValidateButtonClicked(const QString &buttonId)
{
    emit s_openView(Kub3::UI::ViewId::EXPOSURE_SETTINGS_VIEW);
}

void VisualisationView::onSaveButtonClicked(const QString &buttonId)
{
    emit s_openView(Kub3::UI::ViewId::ALIGNMENT_SAVE_PARAMETERS_VIEW);
}

void VisualisationView::onLoadButtonClicked(const QString &buttonId)
{
    emit s_openView(Kub3::UI::ViewId::ALIGNMENT_LOAD_PARAMETERS_VIEW);
}

void VisualisationView::onScreenshotButtonClicked(const QString &buttonId) {}

void VisualisationView::onHardForceContButtonClicked(const QString &buttonId)
{
    bool isHardForceContactFormVisible = m_hardForceContactForm->isVisible();
    m_mapPositionCameras->closeMap();

    if (!isHardForceContactFormVisible)
    {
        leftCamConfigToggled(false);
        rightCamConfigToggled(false);
    }

    // Toggle between the map container and the hard force form
    m_hardForceContactForm->setVisible(!isHardForceContactFormVisible);

    switchColorNavButton(buttonId, isHardForceContactFormVisible);
}

void VisualisationView::onSpeedMotorSubstButtonClicked(const QString &buttonId) {}
void VisualisationView::onSwitchVacumAirButtonClicked(const QString &buttonId) {}
void VisualisationView::onAntiCollisionButtonClicked(const QString &buttonId) {}

void VisualisationView::onVisualMarkButtonClicked(const QString &buttonId)
{
    switchColorNavButton(buttonId, ui->visualMarkLeft->isVisible());
    ui->visualMarkLeft->setVisible(!ui->visualMarkLeft->isVisible());
    ui->visualMarkRight->setVisible(!ui->visualMarkRight->isVisible());
}

void VisualisationView::onMeasurementButtonClicked(const QString &buttonId) {}

// ==========================================
// HELPERS
// ==========================================

void VisualisationView::closeHardForceContactFormIfNeeded(void)
{
    if (m_hardForceContactForm->isVisible())
    {
        m_hardForceContactForm->setVisible(false);
        switchColorNavButton("F", true);
    }
}

void VisualisationView::navButtonToggled(NavButton *button, QWidget *widget)
{
    if (widget)
    {
        button->switchColor(widget->isVisible());
        widget->setVisible(!widget->isVisible());
    }
}
