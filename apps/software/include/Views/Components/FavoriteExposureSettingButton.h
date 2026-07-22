#pragma once

#include <QPushButton>
#include <QString>

class FavoriteExposureSettingButton : public QPushButton
{
    Q_OBJECT

public:
    explicit FavoriteExposureSettingButton(const QString &titleText, const QString &detailsText, QWidget *parent = nullptr);
    FavoriteExposureSettingButton(const FavoriteExposureSettingButton &other);

public:
    QString titleText() const
    {
        return m_titleText;
    }
    QString detailsText() const
    {
        return m_detailsText;
    }

private:
    void setupUi();

    QString m_titleText;
    QString m_detailsText;
};