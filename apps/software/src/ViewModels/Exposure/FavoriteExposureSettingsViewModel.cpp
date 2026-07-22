#include <QJsonObject>
#include <ViewModels/Exposure/FavoriteExposureSettingsViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    FavoriteExposureSettingsViewModel::FavoriteExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        ExposureBaseViewModel(repo, parent)
    {
    }
    FavoriteExposureSettingsViewModel::~FavoriteExposureSettingsViewModel()
    {
    }

    bool FavoriteExposureSettingsViewModel::getAllExposureSettings(QList<FavoriteExposureSettingButton *> &presetButtons, QString *errorMessage)
    {
        QJsonArray presetsArray;
        if (!loadPresetsFromFile(storagePath(), &presetsArray, errorMessage))
        {
            return false;
        }

        for (const QJsonValue &value : presetsArray)
        {
            if (value.isObject())
            {
                QString errorMsg;
                PresetExposure preset = jsonToPreset(value.toObject(), &errorMsg);
                if (!errorMsg.isEmpty())
                {
                    qDebug() << "Error parsing preset: " << errorMsg;
                    continue;
                }
                FavoriteExposureSettingButton *button = new FavoriteExposureSettingButton(preset.name, presetDetailsToStr(preset));
                presetButtons.append(button);
            }
        }
        return true;
    }

} // namespace Kub3::UI::ViewModels::Exposure