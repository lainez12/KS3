#include <pages/LanguageConfigPage.h>

#include <QAbstractItemView>
#include <QLabel>
#include <QList>
#include <QVariant>

namespace Kub3::Components
{

    constexpr const char *locales[] = {LOCALE_EN_US, LOCALE_FR_FR};

    const char *toReadable(const QString &lang)
    {
        if (lang == LOCALE_FR_FR)
            return "Français (France)"; // cocorico
        else if (lang == LOCALE_EN_US)
            return "English (US)";
        return lang.toStdString().c_str(); // Fallback to input
    }

    LanguageConfigPage::LanguageConfigPage(const Kub3::Config::hardware_config_t &conf, QWidget *parent) : QWidget(parent)
    {
        setupUI();
        loadInitialData(conf);
    }

    void LanguageConfigPage::setupUI()
    {
        m_layout = new QVBoxLayout(this);

        auto *header = new QLabel("<b>Locale (Language & Keyboard)</b>");
        header->setObjectName("headerLabel");
        m_layout->addWidget(header);

        // Keyboard/Locale layouts
        auto *widget = new QWidget(this);
        auto *form   = new QFormLayout(widget);
        m_kbSelector = new QComboBox();

        for (QString locale : locales)
        {
            m_kbSelector->addItem(toReadable(locale), QVariant::fromValue(locale));
        }

        m_kbSelector->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        form->addRow("Keyboard layout", m_kbSelector);

        m_layout->addWidget(widget);
        m_layout->addStretch();
    }

    void LanguageConfigPage::loadInitialData(const Kub3::Config::hardware_config_t &conf)
    {
        bool valid = 0;

        for (uint idx = 0; idx < sizeof(locales); ++idx)
        {
            if (conf.keyboardLayout == QString(locales[idx]))
            {
                m_kbSelector->setCurrentIndex(idx);
                valid = 1;
            }
        }

        if (!valid)
        {
            m_kbSelector->addItem(conf.keyboardLayout, conf.keyboardLayout);
        }
    }

    void LanguageConfigPage::pullDataToStruct(Kub3::Config::hardware_config_t &outConf) const
    {
        outConf.keyboardLayout = m_kbSelector->currentData().toString();
    }

} // namespace Components
