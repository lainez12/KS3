#pragma once

#include <QString>
#include <QWidget>
#include <memory>
#include <utils.h>

namespace Ui
{
    class CamLightTestModule;
}

namespace Kub3::Tools::Tester
{

    class CamLightTestModule : public QWidget
    {
        Q_OBJECT

    public:
        explicit CamLightTestModule(const QString &lightId, QWidget *parent = nullptr);
        ~CamLightTestModule() override;

    signals:
        void s_valueChanged(const QString &lightId, uint16_t value);
        void s_toggled(const QString &lightId, bool enabled);

    private:
        Unique<Ui::CamLightTestModule> ui;
        QString m_lightId;
    };

} // namespace Kub3::Tools::Tester
