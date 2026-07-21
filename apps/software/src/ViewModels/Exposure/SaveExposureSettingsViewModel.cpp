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

    SaveExposureSettingsViewModel::~SaveExposureSettingsViewModel()
    {
    }

    bool SaveExposureSettingsViewModel::savePreset(const PresetExposure &preset, QString *errorMessage)
    {
        if (!validatePreset(preset, errorMessage))
            return false;

        const QString path = storagePath();
        if (!ensureParentDirectory(path, errorMessage))
            return false;

        QJsonArray presetsArray;

        if (!loadPresetsFromFile(path, &presetsArray, errorMessage))
        {
            return false;
        }

        bool replacedExistingPreset = presetExistsInFile(presetsArray, preset.name, errorMessage);

        if (!replacedExistingPreset)
            presetsArray.append(presetToJson(preset));

        return savePresetsToFile(path, presetsArray, errorMessage);
    }

    void SaveExposureSettingsViewModel::ps_saveExposureSettings(const PresetExposure &settings)
    {
        m_currentPreset = settings;
    }

    void SaveExposureSettingsViewModel::userConfirmSavePreset(const QString &name)
    {
        m_currentPreset.name = name;
        QString errorMessage;
        if (!savePreset(m_currentPreset, &errorMessage))
        {
        }
    }

    QList<PresetExposure> SaveExposureSettingsViewModel::getAllPresets(QString *errorMessage)
    {
        QJsonArray presetsArray;
        if (!loadPresetsFromFile(storagePath(), &presetsArray, errorMessage))
        {
            return {};
        }

        QList<PresetExposure> presetsList;
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
                presetsList.append(preset);
            }
        }
        return presetsList;
    }

} // namespace Kub3::UI::ViewModels::Exposure