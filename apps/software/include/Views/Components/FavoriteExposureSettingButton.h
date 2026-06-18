#pragma once

#include <QPushButton>
#include <QString>

class FavoriteExposureSettingButton : public QPushButton {
    Q_OBJECT

public:
    explicit FavoriteExposureSettingButton(const QString &titleText, const QString &detailsText, QWidget *parent = nullptr);
};