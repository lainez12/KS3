#pragma once

#include <QImage>
#include <QObject>

#include <HAL/Vision/identifiers.h>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels
{

    class BaseVisionViewModel : public QObject, public BaseViewModel
    {
        Q_OBJECT
    public:
        explicit BaseVisionViewModel(QObject *parent = nullptr);

    public slots:
        void ps_onCameraFrameReceived(const QString &cameraId, const QImage &frame);

    signals:
        void s_upperLeftCameraFrameReady(const QImage &frame);
        void s_upperRightCameraFrameReady(const QImage &frame);

    private:
        std::unordered_map<std::string, void (BaseVisionViewModel::*)(const QImage &)> m_signalsMap;
    };

} // Kub3::UI::ViewModels
