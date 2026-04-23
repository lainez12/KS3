#pragma once

#include <QImage>
#include <QObject>
#include <string>

namespace Kub3::HAL::Vision
{

    class ICamera : public QObject
    {
        Q_OBJECT
    public:
        virtual ~ICamera() = default;

        virtual bool connectDevice(void)    = 0;
        virtual void disconnectDevice(void) = 0;

        virtual void startAcquisition(void) = 0;
        virtual void stopAcquisition(void)  = 0;

        [[nodiscard]] virtual std::string getId() const = 0;

    signals:
        void s_frameReady(const QImage &frame);
        void s_cameraError(QString errorMsg);
    };

} // namespace Kub3::HAL::Vision
