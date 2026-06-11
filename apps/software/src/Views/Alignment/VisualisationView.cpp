#include <QDebug>
#include <QString>
#include <QLabel>

#include <Views/Alignment/VisualisationView.h>
#include <Views/Components/Colors.h>

#include "ui_VisualisationView.h"

#define ID_BTN_BACK     "B"
#define ID_BTN_SAVE     "S"
#define ID_BTN_HOME     "H"
#define ID_BTN_VALIDATE "V"

VisualisationView::VisualisationView(Unique<VisualisationViewModel> viewModel, QWidget *parent) :
    AlignmentViewBase(std::move(viewModel), parent),
    m_keyboard(this),
    ui(new Ui::VisualisationView)
{
    ui->setupUi(this);
    m_keyboard.setupKeyboardConnections(this, "_d");
    m_keyboard.setupKeyboardConnections(this, "_g");
    setDefaultTitleBar("Visualisation");
    createNavButtonsConfigs();
    setNavButtonEnabled(ID_BTN_VALIDATE, true);

    ui->speedCamRight->setup("Cam. Speed", QColor(ORANGE_COLOR), QColor(TURQUOISE_COLOR), ":/icons/speed-motor-low.svg");
    ui->speedCamLeft->setup("Cam. Speed", QColor(ORANGE_COLOR), QColor(TURQUOISE_COLOR), ":/icons/speed-motor-low.svg");
    ui->pickUpRight->setup("Go to X/Y", QColor(ORANGE_COLOR), QColor(TURQUOISE_COLOR), ":/icons/go-to.svg");
    ui->pickUpLeft->setup("Go to X/Y", QColor(ORANGE_COLOR), QColor(TURQUOISE_COLOR), ":/icons/go-to.svg");
    ui->gotoRight->setup("Pick Up X/Y", QColor(ORANGE_COLOR), QColor(TURQUOISE_COLOR), ":/icons/pick-up-xy.svg");
    ui->gotoLeft->setup("Pick Up X/Y", QColor(ORANGE_COLOR), QColor(TURQUOISE_COLOR), ":/icons/pick-up-xy.svg");

    ui->moveCamRight->setup("Move. Cam", QColor(ORANGE_COLOR), QColor(TURQUOISE_COLOR), ":/icons/move-cam.svg");
    ui->moveCamLeft->setup("Move. Cam", QColor(ORANGE_COLOR), QColor(TURQUOISE_COLOR), ":/icons/move-cam.svg");

    ui->configCamLeftFrame->setVisible(false);
    ui->configCamRightFrame->setVisible(false);

    ui->moveLeftCamWidget->setVisible(false);
    ui->moveRightCamWidget->setVisible(false);

    QIcon camIconLeft(":/icons/cam-settings-left.svg");
    QIcon camIconRight(":/icons/cam-settings-right.svg");
    QIcon camIconLeftChecked(":/icons/cam-settings-left-checked.svg");
    QIcon camIconRightChecked(":/icons/cam-settings-right-checked.svg");

    connect(ui->configCamLeftCheck, &QCheckBox::toggled, this, &VisualisationView::leftCamConfigToggled);
    connect(ui->configCamRightCheck, &QCheckBox::toggled, this, &VisualisationView::rightCamConfigToggled);
    
    
    connect(ui->moveCamLeft, &NavButton::clicked, [this]() {
        ui->moveCamLeft->switchColor(ui->moveLeftCamWidget->isVisible());
        ui->moveLeftCamWidget->setVisible(!ui->moveLeftCamWidget->isVisible());
    });
    connect(ui->moveCamRight, &NavButton::clicked, [this]() {
        ui->moveCamRight->switchColor(ui->moveRightCamWidget->isVisible());
        ui->moveRightCamWidget->setVisible(!ui->moveRightCamWidget->isVisible());
    });

    // Create masking distance widget
    m_maskingDistanceWidget = new QWidget(this);
    m_maskingDistanceWidget->setStyleSheet(QString("background-color: %1;").arg(TURQUOISE_COLOR));
    
    m_labelText = new QLabel("Masking distance", m_maskingDistanceWidget);
    m_labelText->setStyleSheet("color: white; font-weight: bold; font-size: 25px;");
    
    m_labelValue = new QLabel("150μm", m_maskingDistanceWidget);
    m_labelValue->setStyleSheet("color: white; font-weight: bold; font-size: 25px;");

    setNewNavButtonsConfigs();
}

VisualisationView::~VisualisationView()
{
}

void VisualisationView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);

    const int BUTTON_SIZE    = 90;
    const int BUTTON_GAP     = 20;
    const int GROUP_DISTANCE = 150;
    const int BOTTOM_MARGIN  = 10;

    // Position buttons in visioLeft
    int visioLeftHeight = ui->visioLeft->height();
    int visioLeftWidth  = ui->visioLeft->width();

    // moveCamLeft: bottom left
    int moveCamLeftX = BOTTOM_MARGIN;
    int moveCamLeftY = visioLeftHeight - BUTTON_SIZE - BOTTOM_MARGIN;
    ui->moveCamLeft->setGeometry(moveCamLeftX, moveCamLeftY, BUTTON_SIZE, BUTTON_SIZE);

    // Other buttons group: 150px to the right of moveCamLeft
    int groupStartX = moveCamLeftX + BUTTON_SIZE + GROUP_DISTANCE;
    int buttonY     = visioLeftHeight - BUTTON_SIZE - BOTTOM_MARGIN;

    ui->speedCamLeft->setGeometry(groupStartX, buttonY, BUTTON_SIZE, BUTTON_SIZE);
    ui->pickUpLeft->setGeometry(groupStartX + BUTTON_SIZE + BUTTON_GAP, buttonY, BUTTON_SIZE, BUTTON_SIZE);
    ui->gotoLeft->setGeometry(groupStartX + 2 * (BUTTON_SIZE + BUTTON_GAP), buttonY, BUTTON_SIZE, BUTTON_SIZE);

    // Position buttons in visioRight
    int visioRightHeight = ui->visioRight->height();
    int visioRightWidth  = ui->visioRight->width();

    // moveCamRight: bottom right
    int moveCamRightX = visioRightWidth - BUTTON_SIZE - BOTTOM_MARGIN;
    int moveCamRightY = visioRightHeight - BUTTON_SIZE - BOTTOM_MARGIN;
    ui->moveCamRight->setGeometry(moveCamRightX, moveCamRightY, BUTTON_SIZE, BUTTON_SIZE);

    // Other buttons group: 150px to the left of moveCamRight
    int groupEndX      = moveCamRightX - GROUP_DISTANCE;
    int gotoRightX     = groupEndX - BUTTON_SIZE;
    int pickUpRightX   = gotoRightX - BUTTON_SIZE - BUTTON_GAP;
    int speedCamRightX = pickUpRightX - BUTTON_SIZE - BUTTON_GAP;

    ui->speedCamRight->setGeometry(speedCamRightX, buttonY, BUTTON_SIZE, BUTTON_SIZE);
    ui->pickUpRight->setGeometry(pickUpRightX, buttonY, BUTTON_SIZE, BUTTON_SIZE);
    ui->gotoRight->setGeometry(gotoRightX, buttonY, BUTTON_SIZE, BUTTON_SIZE);

    const int CONFIG_FRAME_WIDTH = 130; // Icon size (105) + spacer (25)

    // Position configCamLeftFrame: top right corner of visioLeft
    int configLeftFrameHeight = visioLeftHeight - (BUTTON_SIZE * 1.25);
    int configLeftFrameX      = visioLeftWidth - CONFIG_FRAME_WIDTH;
    ui->configCamLeftFrame->setGeometry(configLeftFrameX, 0, CONFIG_FRAME_WIDTH, configLeftFrameHeight);

    // Position configCamRightFrame: top left corner of visioRight
    int configRightFrameHeight = visioRightHeight - (BUTTON_SIZE * 1.25);
    ui->configCamRightFrame->setGeometry(0, 0, CONFIG_FRAME_WIDTH, configRightFrameHeight);

    // Position moveLeftCamWidget: left side of visioLeft, full height
    ui->moveLeftCamWidget->setGeometry(0, 0, ui->moveLeftCamWidget->width(), visioLeftHeight);

    // Position moveRightCamWidget: right side of visioRight, full height
    int moveRightCamWidgetX = visioRightWidth - ui->moveRightCamWidget->width();
    ui->moveRightCamWidget->setGeometry(moveRightCamWidgetX, 0, ui->moveRightCamWidget->width(), visioRightHeight);

    // Position masking distance widget: centered at bottom
    int maskingWidgetWidth = 381;
    int maskingWidgetHeight = 73;
    int maskingWidgetX = (width() - maskingWidgetWidth) / 2;
    int maskingWidgetY = height() - maskingWidgetHeight - BOTTOM_MARGIN;
    m_maskingDistanceWidget->setGeometry(maskingWidgetX, maskingWidgetY, maskingWidgetWidth, maskingWidgetHeight);
    
    // Position labels horizontally inside the widget
    int halfWidth = (maskingWidgetWidth - 30) / 2;
    m_labelText->setGeometry(10, 10, halfWidth, 53);
    m_labelValue->setGeometry(10 + halfWidth + 10, 10, halfWidth, 53);
}

void VisualisationView::setNewNavButtonsConfigs()
{
    NavButtonConfig save(
        "Save",
        ":/icons/save.svg",
        "S",
        std::bind(&VisualisationView::onSaveButtonClicked, this, std::placeholders::_1));
    addNavButton("center", save);

    NavButtonConfig load(
        "Load",
        ":/icons/load.svg",
        "L",
        std::bind(&VisualisationView::onLoadButtonClicked, this, std::placeholders::_1));
    addNavButton("center", load);

    NavButtonConfig screenshot(
        "Screenshot",
        ":/icons/screenshot.svg",
        "P",
        std::bind(&VisualisationView::onScreenshotButtonClicked, this, std::placeholders::_1));
    addNavButton("center", screenshot);

    NavButtonConfig hardForceCont(
        "Hard Force Cont.",
        ":/icons/hard-force-contact.svg",
        "F",
        std::bind(&VisualisationView::onHardForceContButtonClicked, this, std::placeholders::_1));
    addNavButton("center", hardForceCont);

    NavButtonConfig speedMotorSubst(
        "Subst. Speed",
        ":/icons/speed-motor-subst.svg",
        "U",
        std::bind(&VisualisationView::onSpeedMotorSubstButtonClicked, this, std::placeholders::_1));
    addNavButton("center", speedMotorSubst);

    NavButtonConfig maskingDistance(
        "Masking dist.",
        ":/icons/masking-distance.svg",
        "M",
        std::bind(&VisualisationView::onMaskingDistanceButtonClicked, this, std::placeholders::_1));
    addNavButton("center", maskingDistance);

    NavButtonConfig antiCollision(
        "Anti-Collision cam.",
        ":/icons/anti-collision.svg",
        "A",
        std::bind(&VisualisationView::onAntiCollisionButtonClicked, this, std::placeholders::_1));
    addNavButton("center", antiCollision);

    NavButtonConfig visualMark(
        "Visual Mark",
        ":/icons/visual-mark.svg",
        "I",
        std::bind(&VisualisationView::onVisualMarkButtonClicked, this, std::placeholders::_1));
    addNavButton("center", visualMark);

    NavButtonConfig measurement(
        "Measurement",
        ":/icons/measurement.svg",
        "R",
        std::bind(&VisualisationView::onMeasurementButtonClicked, this, std::placeholders::_1));
    addNavButton("center", measurement);
}

void VisualisationView::leftCamConfigToggled(bool checked)
{
    ui->configCamLeftFrame->setVisible(checked);
    if (checked)
    {
        ui->configCamLeftCheck->setIcon(QIcon(":/icons/cam-settings-right-checked.svg"));
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
    }
    else
    {
        ui->configCamRightCheck->setIcon(QIcon(":/icons/cam-settings-right.svg"));
    }
}

void VisualisationView::onBackButtonClicked(const QString &buttonId)
{
}

void VisualisationView::onValidateButtonClicked(const QString &buttonId)
{
    emit s_openView(Kub3::UI::ViewId::EXPOSURE_SETTINGS_VIEW);
}

void VisualisationView::onSaveButtonClicked(const QString &buttonId) {
    emit s_openView(Kub3::UI::ViewId::ALIGNMENT_SAVE_PARAMETERS_VIEW);
}

void VisualisationView::onLoadButtonClicked(const QString &buttonId)
{
}

void VisualisationView::onScreenshotButtonClicked(const QString &buttonId)
{
}

void VisualisationView::onHardForceContButtonClicked(const QString &buttonId)
{
}

void VisualisationView::onSpeedMotorSubstButtonClicked(const QString &buttonId)
{
}

void VisualisationView::onMaskingDistanceButtonClicked(const QString &buttonId)
{
}

void VisualisationView::onAntiCollisionButtonClicked(const QString &buttonId)
{
}

void VisualisationView::onVisualMarkButtonClicked(const QString &buttonId)
{
}

void VisualisationView::onMeasurementButtonClicked(const QString &buttonId)
{
}