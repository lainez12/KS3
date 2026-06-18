#include <QFormLayout>

#include <pages/AdminConfigPage.h>
#include <security/AdminAuthenticator.h>

namespace Kub3::Components
{
    AdminConfigPage::AdminConfigPage(QWidget *parent) : QWidget(parent)
    {
        setupUI();
    }

    void AdminConfigPage::setupUI()
    {
        auto *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(0, 0, 0, 0);

        m_stack = new QStackedWidget(this);

        setupLockScreen();
        setupAdminPanel();

        mainLayout->addWidget(m_stack);

        // Start completely locked
        lock();
    }

    void AdminConfigPage::refreshLockScreenState()
    {
        if (Security::AdminAuthenticator::isVaultInitialized())
        {
            m_lockTitle->setText("<b>Restricted Admin Area</b>");
            m_passwordInput->setPlaceholderText("Enter Administrator Password");
            m_unlockBtn->setText("Unlock");
        }
        else
        {
            m_lockTitle->setText("<b>Factory Setup: Create Admin Password</b>");
            m_passwordInput->setPlaceholderText("Create new password...");
            m_unlockBtn->setText("Set Password & Unlock");
        }
    }

    void AdminConfigPage::setupLockScreen()
    {
        m_lockWidget = new QWidget();
        auto *layout = new QVBoxLayout(m_lockWidget);
        layout->setAlignment(Qt::AlignCenter);

        auto *iconLabel = new QLabel("🔒");
        iconLabel->setStyleSheet("font-size: 48px;");
        iconLabel->setAlignment(Qt::AlignCenter);

        m_lockTitle = new QLabel(""); // Make this a member variable
        m_lockTitle->setStyleSheet("font-size: 18px;");
        m_lockTitle->setAlignment(Qt::AlignCenter);

        m_passwordInput = new QLineEdit();
        m_passwordInput->setEchoMode(QLineEdit::Password);
        m_passwordInput->setMinimumHeight(45);
        m_passwordInput->setMinimumWidth(600);

        m_unlockBtn = new QPushButton("");
        m_unlockBtn->setMinimumHeight(45);
        m_unlockBtn->setMinimumWidth(600);

        m_errorLabel = new QLabel("");
        m_errorLabel->setStyleSheet("color: red; font-weight: bold;");
        m_errorLabel->setAlignment(Qt::AlignCenter);

        layout->addStretch();
        layout->addWidget(iconLabel);
        layout->addWidget(m_lockTitle);
        layout->addWidget(m_passwordInput, 0, Qt::AlignHCenter);
        layout->addWidget(m_unlockBtn, 0, Qt::AlignHCenter);
        layout->addWidget(m_errorLabel);
        layout->addStretch();

        m_stack->addWidget(m_lockWidget);

        connect(m_unlockBtn, &QPushButton::clicked, this, &AdminConfigPage::onUnlockAttempted);
        connect(m_passwordInput, &QLineEdit::returnPressed, this, &AdminConfigPage::onUnlockAttempted);

        // Setup initial text based on vault state
        refreshLockScreenState();
    }

    void AdminConfigPage::setupAdminPanel()
    {
        m_adminContentWidget = new QWidget();
        m_adminLayout        = new QVBoxLayout(m_adminContentWidget);

        auto *header = new QLabel("<b>Admin Configuration</b>");
        header->setStyleSheet("font-size: 14px; margin-bottom: 15px; color: #b30000;");
        m_adminLayout->addWidget(header);

        auto *formLayout = new QFormLayout();

        // The Kloe Mode Checkbox
        m_kloeModeCheckbox = new QCheckBox("Enable Kloe Mode");
        m_kloeModeCheckbox->setStyleSheet("QCheckBox::indicator { width: 24px; height: 24px; }"); // Touch friendly
        m_kloeModeCheckbox->setToolTip("Warning: Alters base machine operational behavior.");

        formLayout->addRow("", m_kloeModeCheckbox);

        m_adminLayout->addLayout(formLayout);
        m_adminLayout->addStretch();

        m_stack->addWidget(m_adminContentWidget);
    }

    void AdminConfigPage::loadInitialData(const Config::admin_config_t &conf)
    {
        // 1. Store the config locally so we can populate the UI later, once unlocked
        m_cachedConf = conf;

        // 2. Only populate UI widgets if the page is currently unlocked
        if (isUnlocked())
        {
            m_kloeModeCheckbox->setChecked(m_cachedConf.kloe_mode);
        }
    }

    void AdminConfigPage::pullDataToStruct(Config::admin_config_t &outConf) const
    {
        // If locked, just return the cached, untouched data.
        if (isUnlocked())
        {
            outConf.kloe_mode = m_kloeModeCheckbox->isChecked();
        }
        else
        {
            outConf = m_cachedConf;
        }
    }

    void AdminConfigPage::onUnlockAttempted()
    {
        QString attempt = m_passwordInput->text();
        bool success    = false;

        if (attempt.isEmpty())
            return;

        if (Security::AdminAuthenticator::isVaultInitialized())
        {
            success = Security::AdminAuthenticator::verifyPassword(attempt);
        }
        else
        {
            // Fail Open strategy: create the vault
            success = Security::AdminAuthenticator::setNewPassword(attempt);
        }

        if (success)
        {
            m_unlocked = true;
            m_errorLabel->clear();
            m_passwordInput->clear();
            m_stack->setCurrentWidget(m_adminContentWidget);

            emit adminUnlocked(); // Tells ConfigWindow it's safe to load the data into the UI now
        }
        else
        {
            m_errorLabel->setText("Invalid Password / Setup Failed.");
            m_passwordInput->clear();
        }
    }

    bool AdminConfigPage::isUnlocked() const
    {
        return m_unlocked;
    }

    void AdminConfigPage::lock()
    {
        m_unlocked = false;
        m_passwordInput->clear();
        m_errorLabel->clear();
        refreshLockScreenState();
        m_stack->setCurrentWidget(m_lockWidget);

        // Strip data from UI to prevent visual leakage via Qt Inspector tools
        m_kloeModeCheckbox->setChecked(false);
    }

} // namespace Kub3::Components
