#pragma once

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

#include <Config/conf.h>

namespace Kub3::Components
{

    class ForceConversionsConfigPage : public QWidget
    {
        Q_OBJECT

    public:
        explicit ForceConversionsConfigPage(const Config::hardware_config_t &conf, QWidget *parent = nullptr);

        void pullDataToStruct(Config::hardware_config_t &out) const;

    private:
        void setupUI();
        void loadInitialData(const Config::hardware_config_t &conf);

    private:
        QVBoxLayout *m_layout = nullptr;

        QDoubleSpinBox *m_leftADCToGFRatio  = nullptr;
        QDoubleSpinBox *m_rightADCToGFRatio = nullptr;
        QDoubleSpinBox *m_backADCToGFRatio  = nullptr;
    };

}
