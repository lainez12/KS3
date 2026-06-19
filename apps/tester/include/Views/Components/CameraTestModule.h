#pragma once

#include <QRect>
#include <QString>
#include <QWidget>

#include <Views/Components/CameraStreamWidget.h>
#include <utils.h>

namespace Ui
{
    class CameraTestModule;
}

namespace Kub3::Tools::Tester
{
    class CameraTestModule : public QWidget
    {
        Q_OBJECT

    public:
        explicit CameraTestModule(const QString &cameraId,
                                  const QString &title,
                                  const Optional<QString> &focalId = std::nullopt,
                                  const Optional<QString> &lightId = std::nullopt,
                                  QWidget *parent                  = nullptr);
        ~CameraTestModule() override;

        CameraStreamWidget *streamWidget() const;

    signals:
        void s_exposureChanged(const QString &camId, double val);
        void s_gainChanged(const QString &camId, double val);
        void s_framerateChanged(const QString &camId, double val);
        void s_zoomChanged(const QString &camId, double val);
        void s_roiChanged(const QString &camId, QRect roi);

        void s_focalToggled(const QString &focalId, bool en);
        void s_lightToggled(const QString &lightId, bool en);
        void s_focalChanged(const QString &focalId, double val);
        void s_lightChanged(const QString &lightId, double val);

    private slots:
        void onROIFieldChanged();

    private:
        void setupComponents();

        Ui::CameraTestModule *ui;
        QString m_cameraId;
        Optional<QString> m_focalId = std::nullopt;
        Optional<QString> m_lightId = std::nullopt;
    };
}
