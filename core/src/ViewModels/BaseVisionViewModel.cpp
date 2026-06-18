#include <QDebug>
#include <ViewModels/BaseVisionViewModel.h>

namespace Kub3::UI::ViewModels
{
    BaseVisionViewModel::BaseVisionViewModel(QObject *parent) : QObject(parent)
    {
        m_signalsMap.insert({UPPER_LEFT_CAMERA, &BaseVisionViewModel::s_upperLeftCameraFrameReady});
        m_signalsMap.insert({UPPER_RIGHT_CAMERA, &BaseVisionViewModel::s_upperRightCameraFrameReady});
    }

    void BaseVisionViewModel::ps_onCameraFrameReceived(const QString &cameraId, const QImage &frame)
    {
        if (auto it = m_signalsMap.find(cameraId.toStdString()); it != m_signalsMap.end())
            emit(this->*(it->second))(frame); // Emitting the signal matching the cameraId with the frame as argument
        else
            qWarning() << "[BaseVisionViewModel::ps_onCameraFrameReceived] Signal not found";
    }
}
