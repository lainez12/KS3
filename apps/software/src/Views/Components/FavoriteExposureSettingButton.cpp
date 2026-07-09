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

    titleLabel->setProperty("class", "color-blue text-medium title-favorite-exposure");
    detailsLabel->setProperty("class", "color-orange text-small details-favorite-exposure");

    detailsLabel->setWordWrap(true);
    detailsLabel->setAlignment(Qt::AlignJustify);
    layout->addWidget(titleLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    layout->addStretch();
    layout->addWidget(detailsLabel, 0, Qt::AlignRight | Qt::AlignVCenter);

    layout->setContentsMargins(20, 6, 20, 6);

    this->setProperty("class", "favorite-exposure-button");

    this->setMinimumHeight(90);
    this->setMaximumHeight(130);
}