#pragma once

#include <QString>
#include <QWidget>
#include <memory>
#include <utils.h>

namespace Ui
{
    class FocalTestModule;
}

namespace Kub3::Tools::Tester
{

    class FocalTestModule : public QWidget
    {
        Q_OBJECT

    public:
        explicit FocalTestModule(const QString &focalId, QWidget *parent = nullptr);
        ~FocalTestModule() override;

    signals:
        void s_valueChanged(const QString &focalId, uint16_t value);
        void s_toggled(const QString &focalId, bool enabled);

    private:
        Unique<Ui::FocalTestModule> ui;
        QString m_focalId;
    };

} // namespace Kub3::Tools::Tester
