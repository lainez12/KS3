#include "ui_ProgressExposureView.h"
#include <QShowEvent>
#include <QString>
#include <Views/Components/Colors.h>
#include <Views/Exposure/ProgressExposureView.h>

#define ID_BTN_BACK     "B"
#define ID_BTN_SAVE     "S"
#define ID_BTN_VALIDATE "V"
#define ID_BTN_HOME     "H"

ProgressExposureView::ProgressExposureView(Unique<ProgressExposureViewModel> viewModel, QWidget *parent) :
    ExposureViewBase(std::move(viewModel), parent),
    ui(new Ui::ProgressExposureView)
{
    ui->setupUi(this);

    m_progressTimer.setInterval(10);
    connect(&m_progressTimer, &QTimer::timeout, this, &ProgressExposureView::updateProgressBar);

    m_tempTimer.setInterval(200);
    connect(&m_tempTimer, &QTimer::timeout, this, &ProgressExposureView::updateTemp);

    createNavButtonsConfigs();
    setDefaultTitleBar("Exposure in progress");
    setNavButtonEnabled(ID_BTN_HOME, false);
}
ProgressExposureView::~ProgressExposureView()
{
}

void ProgressExposureView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
}

void ProgressExposureView::showEvent(QShowEvent *event)
{
    setNavButtonEnabled(ID_BTN_VALIDATE, false);
    vm = getViewModel<ProgressExposureViewModel>();
    if (!vm)
    {
        ExposureViewBase::showEvent(event);
        return;
    }
    PresetExposure preset = vm->getCurrentPreset();
    ui->exposureModeLabel->setText(vm->modeToString(preset.mode) + " exposure");
    ui->detailsExposureLabel->setText(vm->presetDetailsToStr(preset));

    m_durationInMS = preset.getDurationInMS();
    ui->progressBar->setRange(0, m_durationInMS);
    ui->progressBar->setValue(0);

    m_progressTimer.stop();
    if (m_durationInMS > 0)
    {
        m_progressTimer.start();
        m_tempTimer.start();
        m_elapsedTimer.restart();
    }

    ExposureViewBase::showEvent(event);
    vm->ui_launchExposure();
}

void ProgressExposureView::updateProgressBar()
{
    const qint64 elapsedTime = m_elapsedTimer.elapsed();

    if (elapsedTime >= m_durationInMS)
    {
        ui->progressBar->setValue(m_durationInMS);
        m_progressTimer.stop();
        m_tempTimer.stop();
        setNavButtonEnabled(ID_BTN_VALIDATE, true);
        setNavButtonEnabled(ID_BTN_HOME, true);
        return;
    }

    ui->progressBar->setValue(elapsedTime);
    int min = (m_durationInMS - elapsedTime) / 60000;
    int sec = ((m_durationInMS - (min * 60000)) - elapsedTime) / 1000;
    ui->remainTimeLabel->setText(QString::number(min) + ":" + QString::number(sec, 10).rightJustified(2, '0'));
}

void ProgressExposureView::updateTemp()
{
    double temp = vm->getTemperature();
    ui->tempLedLabel->setText(QString("%1°C").arg(QString::number(temp, 'f', 1)));
}

void ProgressExposureView::onBackButtonClicked()
{
    // emit s_openView(Kub3::UI::ViewId::BACK_VIEW);
}

void ProgressExposureView::onValidateButtonClicked()
{
    emit s_openView(Kub3::UI::ViewId::COMPLETE_EXPOSURE_VIEW);
}