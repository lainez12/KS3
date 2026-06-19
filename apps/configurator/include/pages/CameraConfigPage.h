#pragma once

#include <QWidget>

#include <Config/conf.h>

namespace Ui
{
    class CameraConfigPage;
}

namespace Kub3::Components
{

    class CameraConfigPage : public QWidget
    {
        Q_OBJECT

    public:
        explicit CameraConfigPage(const Kub3::Config::camera_config_t &conf, QWidget *parent = nullptr);
        ~CameraConfigPage() override;

        // Pulls the UI data back into the struct. Note: `outConf.id` is left untouched.
        void pullDataToStruct(Kub3::Config::camera_config_t &outConf) const;

    private:
        void setupUI(const Kub3::Config::camera_config_t &conf);
        void loadInitialData(const Kub3::Config::camera_config_t &conf);

    private:
        Ui::CameraConfigPage *ui;
    };

} // namespace Kub3::Components