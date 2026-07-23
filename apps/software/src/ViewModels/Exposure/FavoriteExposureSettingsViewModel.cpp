#include <QJsonObject>
#include <ViewModels/Exposure/FavoriteExposureSettingsViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    FavoriteExposureSettingsViewModel::FavoriteExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        ExposureBaseViewModel(repo, parent)
    {
    }

    bool FavoriteExposureSettingsViewModel::getAllExposureSettings(QList<FavoriteExposureSettingButton *> &presetButtons, QString *errorMessage)
    {
        QJsonArray presetsArray;

        if (!loadPresetsFromFile(storagePath(), &presetsArray, errorMessage))
        {
            emit s_createPopUpWithText("Error loading favorite settings", {{"OK", []() {}}}, *errorMessage);
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
                    qCritical() << "Error parsing exposure preset: " << errorMsg;
                    continue;
                }
                FavoriteExposureSettingButton *button = new FavoriteExposureSettingButton(preset.name, presetDetailsToStr(preset));
                presetButtons.append(button);
            }
        }
        return true;
    }

    bool FavoriteExposureSettingsViewModel::uiLoadExposurePreset(QString &presetName)
    {
        QJsonArray presetsArray;
        QString errorMessage;

        if (!loadPresetsFromFile(storagePath(), &presetsArray, &errorMessage))
        {
            qCritical() << "Failed to load preset: " << errorMessage;
            emit s_createPopUpWithText("Error", {{"OK", []() {}}}, "Failed to apply the selected preset.");
            return false;
        }

        PresetExposure chosenPreset = getPresetByName(presetsArray, presetName, &errorMessage);

        if (chosenPreset.name.isEmpty())
        {
            qCritical() << "Exposure preset not found: " << presetName;
            emit s_createPopUpWithText("Error", {{"OK", []() {}}}, "Failed to apply the selected preset.");
            return false;
        }

        emit s_exposurePresetLoaded(chosenPreset);
        return true;
    }

} // namespace Kub3::UI::ViewModels::Exposure