#pragma once

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

#include <Config/machine_config.h>

namespace Kub3::Components
{

    class ForceConfigPage : public QWidget
    {
        Q_OBJECT

    public:
        explicit ForceConfigPage(const Kub3::Config::process_config_t &conf, QWidget *parent = nullptr);

        void pullDataToStruct(Kub3::Config::process_config_t &outConf) const;

    private:
        void setupUI();
        void loadInitialData(const Kub3::Config::process_config_t &conf);

    private:
        QVBoxLayout *m_layout = nullptr;

        QDoubleSpinBox *m_hwCrashForce     = nullptr;
        QDoubleSpinBox *m_maxForce         = nullptr;
        QDoubleSpinBox *m_contactThreshold = nullptr;
    };

}
