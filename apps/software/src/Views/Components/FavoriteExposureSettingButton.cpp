#include <QHBoxLayout>
#include <QLabel>

#include <Views/Components/FavoriteExposureSettingButton.h>

FavoriteExposureSettingButton::FavoriteExposureSettingButton(const QString &titleText, const QString &detailsText, QWidget *parent) : QPushButton(parent), m_titleText(titleText), m_detailsText(detailsText)
{
    setupUi();
}

FavoriteExposureSettingButton::FavoriteExposureSettingButton(const FavoriteExposureSettingButton &other) : QPushButton(other.parentWidget()), m_titleText(other.m_titleText), m_detailsText(other.m_detailsText)
{
    setStyleSheet(other.styleSheet());
    setMinimumHeight(other.minimumHeight());
    setupUi();
}

void FavoriteExposureSettingButton::setupUi()
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    QLabel *titleLabel   = new QLabel(m_titleText, this);
    QLabel *detailsLabel = new QLabel(m_detailsText, this);

    titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    detailsLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    titleLabel->setStyleSheet("color: #0078D7; font-size: 25px; font-weight: bold; text-decoration: underline; border: none;");
    detailsLabel->setStyleSheet("color: #E25B22; font-size: 20px; font-weight: bold; border: none;");

    detailsLabel->setWordWrap(true);

    layout->addWidget(titleLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    layout->addStretch();
    layout->addWidget(detailsLabel, 0, Qt::AlignRight | Qt::AlignVCenter);

    layout->setContentsMargins(8, 4, 8, 4);

    this->setStyleSheet(
        "QPushButton {"
        "   border: 1px solid #75B2E5;"
        "   background-color: transparent;"
        "}"
        "QPushButton:hover {"
        "   background-color: #E6F0FA;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #CCE4F7;"
        "}");

    this->setMinimumHeight(70);
}