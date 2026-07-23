#include <QDebug>
#include <ViewModels/BaseVisionViewModel.h>

namespace Kub3::UI::ViewModels
{
    BaseVisionViewModel::BaseVisionViewModel(QObject *parent) : BaseViewModel(parent)
    {
    }

    void BaseVisionViewModel::ps_onCameraFrameReceived(const QString &cameraId, const QImage &frame)
    {
        emit s_frameReady(cameraId, frame);
    }
}