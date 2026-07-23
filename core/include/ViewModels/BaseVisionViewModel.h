#pragma once

#include <QImage>
#include <QObject>

#include <HAL/Vision/identifiers.h>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels
{

    class BaseVisionViewModel : public BaseViewModel
    {
        Q_OBJECT

    public:
        explicit BaseVisionViewModel(QObject *parent = nullptr);

    public slots:
        void ps_onCameraFrameReceived(const QString &cameraId, const QImage &frame);

    signals:
        // Generic signal emitting the camera ID alongside the frame
        void s_frameReady(const QString &cameraId, const QImage &frame);
    };

} // Kub3::UI::ViewModels
