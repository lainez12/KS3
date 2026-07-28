#pragma once

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

#include <Config/conf.h>

namespace Kub3::Components
{

    class DrawersPositionsPage : public QWidget
    {
        Q_OBJECT

    public:
        explicit DrawersPositionsPage(const Config::drawer_process_config_t &conf, QWidget *parent = nullptr);

        void pullDataToStruct(Config::drawer_process_config_t &outConf) const;

    private:
        void setupUI();
        void loadInitialData(const Config::drawer_process_config_t &conf);

    private:
        QVBoxLayout *m_layout = nullptr;

        QDoubleSpinBox *m_cm3Reset               = nullptr;
        QDoubleSpinBox *m_maskEjectSlowingPosMm  = nullptr;
        QDoubleSpinBox *m_waferEjectSlowingPosMm = nullptr;
    };

} // namespace Kub3::Components
