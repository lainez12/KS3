#pragma once

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

#include <Config/conf.h>

namespace Kub3::Components
{

    class AlignmentPositionsPage : public QWidget
    {
        Q_OBJECT

    public:
        explicit AlignmentPositionsPage(const Config::alignment_process_config_t &conf, QWidget *parent = nullptr);

        void pullDataToStruct(Config::alignment_process_config_t &outConf) const;

    private:
        void setupUI();
        void loadInitialData(const Config::alignment_process_config_t &conf);

    private:
        QVBoxLayout *m_layout = nullptr;

        QDoubleSpinBox *m_xCenterMm     = nullptr;
        QDoubleSpinBox *m_yCenterMm     = nullptr;
        QDoubleSpinBox *m_thetaCenterMm = nullptr;
    };

} // namespace Kub3::Components
