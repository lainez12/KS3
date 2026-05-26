#include "FavoriteExposureSettingButton.h"
#include <QHBoxLayout>
#include <QLabel>

FavoriteExposureSettingButton::FavoriteExposureSettingButton(const QString &titleText, const QString &detailsText, QWidget *parent) : QPushButton(parent) {

    QHBoxLayout *layout = new QHBoxLayout(this);

    QLabel *titleLabel   = new QLabel(titleText, this);
    QLabel *detailsLabel = new QLabel(detailsText, this);

    titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    detailsLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    titleLabel->setStyleSheet("color: #0078D7; font-size: 18px; font-weight: bold; text-decoration: underline; border: none;");
    detailsLabel->setStyleSheet("color: #E25B22; font-size: 14px; font-weight: bold; border: none;");

    detailsLabel->setWordWrap(true);

    layout->addWidget(titleLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    layout->addStretch();
    layout->addWidget(detailsLabel, 0, Qt::AlignRight | Qt::AlignVCenter);

    layout->setContentsMargins(20, 10, 20, 10);

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