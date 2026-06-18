#pragma once

#include <QImage>
#include <QPaintEvent>
#include <QWidget>

class CameraStreamWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CameraStreamWidget(QWidget *parent = nullptr);
    ~CameraStreamWidget() override = default;

public slots:
    // Slot to receive the frame from the ViewModel
    void ps_onFrameUpdated(const QImage &frame);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QImage m_currentFrame;
    bool m_hasFrame = false; // if false, we can display a placeholder
};
