#pragma once

#include <QEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QWidget>

namespace Ui
{
    class RealPositionCameras;
} // namespace Ui

class RealPositionCameras : public QWidget
{
    Q_OBJECT
public:
    explicit RealPositionCameras(QWidget *parent = nullptr);

protected:
public slots:
    void openMap(void);
    void closeMap(void);

private slots:
    void onBtnOpenCloseToggled(bool checked);

signals:
    void s_openMap(void);
    void s_closeMap(void);

private:
    Ui::RealPositionCameras *ui;
    QString m_text;
};
