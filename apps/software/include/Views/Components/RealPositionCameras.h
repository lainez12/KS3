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
    ~RealPositionCameras() override;

public:
    void openMap(void);
    void closeMap(void);

signals:
    void s_openMap(void);
    void s_closeMap(void);

protected:
    void resizeEvent(QResizeEvent *ev) override;

private slots:
    void onBtnOpenCloseToggled(bool checked);

private:
    Ui::RealPositionCameras *ui;
    QString m_text;
    QPixmap m_icon;
};
