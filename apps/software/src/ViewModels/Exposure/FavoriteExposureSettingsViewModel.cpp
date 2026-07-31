#include <QJsonObject>
#include <ViewModels/Exposure/FavoriteExposureSettingsViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    FavoriteExposureSettingsViewModel::FavoriteExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        ExposureBaseViewModel(repo, parent)
    {
    }

    Result<QList<FavoriteExposureSettingButton *>, QString> FavoriteExposureSettingsViewModel::getAllExposureSettings()
    {
        QJsonArray presetsArray;
        auto loadRes = loadPresetsFromFile(storagePath(), &presetsArray);

        if (loadRes.is_err())
        {
            emit s_createPopUpWithText("Error loading favorite settings", {{"OK", []() {}}}, loadRes.unwrap_err());
            return Err(loadRes.unwrap_err());
        }

        QList<FavoriteExposureSettingButton *> presetButtons;
        for (const QJsonValue &value : presetsArray)
        {
            if (value.isObject())
            {
                auto presetRes = jsonToPreset(value.toObject());

                if (presetRes.is_err())
                {
                    qCritical() << "Error parsing exposure preset:" << presetRes.unwrap_err();
                    continue;
                }

                PresetExposure preset                 = presetRes.unwrap();
                FavoriteExposureSettingButton *button = new FavoriteExposureSettingButton(preset.name, presetDetailsToStr(preset));
                connect(button, &FavoriteExposureSettingButton::deleteButtonClicked, this, [this, preset, button]() {
                    emit s_createPopUpWithText("Delete Favorite Setting", {{"Yes", [this, preset, button]() {
                                                                                deleteExposurePreset(preset.name);
                                                                                button->setVisible(false);
                                                                                emit s_closePopUp();
                                                                            }},
                                                                           {"No", [this]() {
                                                                                emit s_closePopUp();
                                                                            }}},
                                               "Are you sure you want to delete '" + preset.name + "'?");
                });
                presetButtons.append(button);
            }
        }
        return Ok(presetButtons);
    }

    Result<Unit, QString> FavoriteExposureSettingsViewModel::uiLoadExposurePreset(const QString &presetName)
    {
        QJsonArray presetsArray;
        auto loadRes = loadPresetsFromFile(storagePath(), &presetsArray);

        if (loadRes.is_err())
        {
            qCritical() << "Failed to load preset file:" << loadRes.unwrap_err();
            emit s_createPopUpWithText("Error", {{"OK", []() {}}}, "Failed to apply the selected preset.");
            return Err(loadRes.unwrap_err());
        }

        auto chosenRes = getPresetByName(presetsArray, presetName);

        if (chosenRes.is_err())
        {
            qCritical() << "Exposure preset not found:" << presetName << "(" << chosenRes.unwrap_err() << ")";
            emit s_createPopUpWithText("Error", {{"OK", []() {}}}, "Failed to apply the selected preset.");
            return Err(chosenRes.unwrap_err());
        }

        emit s_exposurePresetLoaded(chosenRes.unwrap());
        return Ok<Unit>({});
    }

    void FavoriteExposureSettingsViewModel::deleteExposurePreset(const QString &presetName)
    {
        QJsonArray presetsArray;
        auto loadRes = loadPresetsFromFile(storagePath(), &presetsArray);

        if (loadRes.is_err())
        {
            qCritical() << "Failed to load preset file:" << loadRes.unwrap_err();
            emit s_createPopUpWithText("Error", {{"OK", []() {}}}, "Failed to delete the selected preset.");
            return;
        }

        if (!deleteByName(presetsArray, presetName))
        {
            qCritical() << "Failed to delete preset:" << presetName;
            emit s_createPopUpWithText("Error", {{"OK", []() {}}}, "Failed to delete the selected preset.");
            return;
        }

        auto saveRes = savePresetsToFile(storagePath(), presetsArray);
        if (saveRes.is_err())
        {
            qCritical() << "Failed to save preset file after deletion:" << saveRes.unwrap_err();
            emit s_createPopUpWithText("Error", {{"OK", []() {}}}, "Failed to delete the selected preset.");
        }
    }

} // namespace Kub3::UI::ViewModels::Exposure
