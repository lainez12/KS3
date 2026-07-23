#include <ViewModels/Exposure/SaveExposureSettingsViewModel.h>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QTextStream>

namespace Kub3::UI::ViewModels::Exposure
{
    SaveExposureSettingsViewModel::SaveExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        ExposureBaseViewModel(repo, parent)
    {
    }

    Result<Unit, QString> SaveExposureSettingsViewModel::savePreset(const PresetExposure &preset, bool replaceExisting)
    {
        auto validRes = validatePreset(preset);
        if (validRes.is_err())
            return Err(QString(validRes.unwrap_err()));

        const QString path = storagePath();

        auto dirRes = ensureParentDirectory(path);
        if (dirRes.is_err())
            return Err(dirRes.unwrap_err());

        QJsonArray presetsArray;
        auto loadRes = loadPresetsFromFile(path, &presetsArray);
        if (loadRes.is_err())
            return Err(loadRes.unwrap_err());

        bool replacedExistingPreset = presetExistsInFile(presetsArray, preset.name);

        if (replacedExistingPreset && !replaceExisting)
        {
            return Err(QStringLiteral("A preset with the same name already exists. Please choose a different name or enable replacement."));
        }

        if (!replacedExistingPreset)
            presetsArray.append(presetToJson(preset));
        else
            replaceExistingPreset(presetsArray, preset);

        return savePresetsToFile(path, presetsArray);
    }

    void SaveExposureSettingsViewModel::ps_saveExposureSettings(const PresetExposure &settings)
    {
        m_currentPreset = settings;
    }

    void SaveExposureSettingsViewModel::userConfirmSavePreset(const QString &name)
    {
        m_currentPreset.name = name;

        auto saveRes = savePreset(m_currentPreset);
        if (saveRes.is_err())
        {
            emit s_errorSavingPreset(saveRes.unwrap_err());
            return;
        }
        emit s_presetSaved();
    }

    Result<QList<PresetExposure>, QString> SaveExposureSettingsViewModel::getAllPresets()
    {
        QJsonArray presetsArray;
        auto loadRes = loadPresetsFromFile(storagePath(), &presetsArray);

        if (loadRes.is_err())
            return Err(loadRes.unwrap_err());

        QList<PresetExposure> presetsList;
        for (const QJsonValue &value : presetsArray)
        {
            if (value.isObject())
            {
                auto presetRes = jsonToPreset(value.toObject());
                if (presetRes.is_err())
                {
                    qCritical() << "Error parsing preset:" << presetRes.unwrap_err();
                    continue;
                }
                presetsList.append(presetRes.unwrap());
            }
        }
        return Ok(presetsList);
    }

} // namespace Kub3::UI::ViewModels::Exposure
