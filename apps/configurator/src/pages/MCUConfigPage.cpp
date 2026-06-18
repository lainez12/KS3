#include <pages/MCUConfigPage.h>

#include <QAbstractItemView>
#include <QLabel>
#include <QList>
#include <QVariant>

namespace Kub3::Components
{

    MCUConfigPage::MCUConfigPage(const Kub3::Config::hardware_config_t &conf, QWidget *parent) : QWidget(parent)
    {
        setupUI();
        loadInitialData(conf);
    }

    void MCUConfigPage::setupUI()
    {
        m_layout = new QVBoxLayout(this);

        auto *header = new QLabel("<b>Micro-controllers (MCU) Communication</b>");
        header->setObjectName("headerLabel");
        m_layout->addWidget(header);

        // Standard UART Baud Rates
        const QList<uint32_t> standardBauds = {9600, 19200, 38400, 57600, 115200, 230400};

        for (int i = 0; i < MCU_COUNT; ++i)
        {
            auto *group = new QGroupBox(QString("Micro-controller Node %1").arg(i + 1));
            auto *form  = new QFormLayout(group);

            m_mcuUIs[i].portInput    = new QLineEdit();
            m_mcuUIs[i].baudSelector = new QComboBox();

            m_mcuUIs[i].portInput->setPlaceholderText("e.g. /dev/ttyACM0 or /dev/arduino1");
            for (uint32_t baud : standardBauds)
                m_mcuUIs[i].baudSelector->addItem(QString::number(baud), QVariant::fromValue(baud));

            // Ensure the dropdown list inherits our touch-friendly QSS sizing
            m_mcuUIs[i].baudSelector->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

            form->addRow("Serial Port:", m_mcuUIs[i].portInput);
            form->addRow("Baud Rate:", m_mcuUIs[i].baudSelector);

            m_layout->addWidget(group);
        }

        m_layout->addStretch();
    }

    void MCUConfigPage::loadInitialData(const Kub3::Config::hardware_config_t &conf)
    {
        for (int i = 0; i < MCU_COUNT; ++i)
        {
            m_mcuUIs[i].portInput->setText(conf.mcus[i].port);

            // Attempt to find the configured baud rate in our standard list
            int idx = m_mcuUIs[i].baudSelector->findData(conf.mcus[i].baudrate);
            if (idx != -1)
            {
                m_mcuUIs[i].baudSelector->setCurrentIndex(idx);
            }
            else
            {
                // Failsafe: If the .ini has an unusual custom baud rate, inject it dynamically so it isn't lost
                m_mcuUIs[i].baudSelector->addItem(QString::number(conf.mcus[i].baudrate), conf.mcus[i].baudrate);
                m_mcuUIs[i].baudSelector->setCurrentIndex(m_mcuUIs[i].baudSelector->count() - 1);
            }
        }
    }

    void MCUConfigPage::pullDataToStruct(Kub3::Config::hardware_config_t &outConf) const
    {
        for (int i = 0; i < MCU_COUNT; ++i)
        {
            outConf.mcus[i].port     = m_mcuUIs[i].portInput->text();
            outConf.mcus[i].baudrate = m_mcuUIs[i].baudSelector->currentData().toUInt();
        }
    }

} // namespace Components
