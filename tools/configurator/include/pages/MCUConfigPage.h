#pragma once

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QWidget>

#include <Config/conf.h>

namespace Kub3::Components
{

    class MCUConfigPage : public QWidget
    {
        Q_OBJECT

    public:
        explicit MCUConfigPage(const Kub3::Config::hardware_config_t &conf, QWidget *parent = nullptr);

        void pullDataToStruct(Kub3::Config::hardware_config_t &outConf) const;

    private:
        void setupUI();
        void loadInitialData(const Kub3::Config::hardware_config_t &conf);

    private:
        QVBoxLayout *m_layout = nullptr;

        struct McuUI {
            QLineEdit *portInput;
            QComboBox *baudSelector;
        };

        McuUI m_mcuUIs[MCU_COUNT];
    };

} // namespace Kub3::Components
