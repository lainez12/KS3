#pragma once

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include <Config/conf.h>

namespace Kub3::Components
{

    class MotorConfigPage : public QWidget
    {
        Q_OBJECT

    public:
        explicit MotorConfigPage(const Kub3::Config::motor_config_t &hwConf,
                                 const Kub3::Config::KinematicProfiles &kinConf,
                                 const QStringList &allMotorIds,
                                 QWidget *parent = nullptr);

        // Pulls both HW and Process data back into the main structs
        void pullDataToStruct(Kub3::Config::motor_config_t &outHw,
                              Kub3::Config::KinematicProfiles &outKin) const;

    signals:
        void profileExportRequested(const QString &targetMotorId, const Kub3::Config::kinematic_profile_t &profile);

    private slots:
        // Hardware slots
        void onHardwareTypeChanged(int index);
        // Kinematics slots
        void onAddProfile();
        void onRemoveProfile();
        void onExportProfile();
        void onProfileSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);
        void stepperRecomputeFullStepSize();

    private:
        void setupUI(const QString &motorId);
        void setupHardwareTab();
        void setupKinematicsTab();

        void loadInitialData(const Kub3::Config::motor_config_t &hwConf);

        // Kinematic Data Flow
        void refreshProfileList();
        void loadProfileForm(const QString &profileId);
        void saveCurrentProfileForm(const QString &profileId);

    private:
        QVBoxLayout *m_layout = nullptr;
        QTabWidget *m_tabs    = nullptr;

        // --- HARDWARE UI ---
        QComboBox *m_typeSelector   = nullptr;
        QStackedWidget *m_typeStack = nullptr;

        QSpinBox *m_stepperStepsPerRev      = nullptr;
        QDoubleSpinBox *m_stepperScrewPitch = nullptr;
        QDoubleSpinBox *m_stepperMaxVel     = nullptr;
        QDoubleSpinBox *m_stepperMaxAcc     = nullptr;
        QSpinBox *m_stepperEncTops          = nullptr;
        QLabel *m_stepperFullStepSizeRO     = nullptr; // RO at the end for read only

        QDoubleSpinBox *m_dcScrewPitch = nullptr;
        QDoubleSpinBox *m_dcMaxVel     = nullptr;
        QDoubleSpinBox *m_dcMaxAcc     = nullptr;
        QSpinBox *m_dcEncTops          = nullptr;

        // --- KINEMATICS UI ---
        QListWidget *m_profileList   = nullptr;
        QWidget *m_profileFormWidget = nullptr;

        QDoubleSpinBox *m_kinTargetVel = nullptr;
        QDoubleSpinBox *m_kinAcc       = nullptr;

        QLabel *m_kinParamsTitle         = nullptr;
        QStackedWidget *m_kinParamsStack = nullptr;
        QComboBox *m_kinStepFraction     = nullptr; // For Steppers

        // --- LOCAL DATA ---
        Kub3::Config::KinematicProfiles m_profiles; // Local RAM copy for infinite profiles
        QStringList m_allMotorIds;
        QString m_currentMotorId;
    };

} // Kub3::Components