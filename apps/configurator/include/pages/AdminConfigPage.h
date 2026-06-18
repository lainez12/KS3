#pragma once

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

#include <Config/admin.h>

namespace Kub3::Components
{
    class AdminConfigPage : public QWidget
    {
        Q_OBJECT

    public:
        explicit AdminConfigPage(QWidget *parent = nullptr);

        // Tells the ConfigWindow if we are allowed to pull data from this page
        bool isUnlocked() const;
        // Forces the page to lock
        void lock();

        void loadInitialData(const Config::admin_config_t &conf);
        void pullDataToStruct(Config::admin_config_t &outConf) const;

    signals:
        // Emitted when the user successfully unlocks, so ConfigWindow can load the admin struct
        void adminUnlocked();

    private slots:
        void onUnlockAttempted();

    private:
        void refreshLockScreenState();
        void setupUI();
        void setupLockScreen();
        void setupAdminPanel();

    private:
        QStackedWidget *m_stack = nullptr;

        // --- Lock Screen UI ---
        QLabel *m_lockTitle        = nullptr;
        QWidget *m_lockWidget      = nullptr;
        QLineEdit *m_passwordInput = nullptr;
        QPushButton *m_unlockBtn   = nullptr;
        QLabel *m_errorLabel       = nullptr;

        // --- Admin Data UI ---
        QWidget *m_adminContentWidget = nullptr;
        QVBoxLayout *m_adminLayout    = nullptr;
        QCheckBox *m_kloeModeCheckbox = nullptr;

        bool m_unlocked = false;
        Config::admin_config_t m_cachedConf;
    };
}
