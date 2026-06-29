#pragma once

#include <QFrame>
#include <QLabel>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPixmap>
#include <QString>
#include <QWidget>

class ScreenshotThumbnailWidget final : public QFrame
{
public:
    ScreenshotThumbnailWidget(const QString &filePath, QWidget *parent = nullptr);

    void setSelected(bool selected);
    bool isSelected() const;
    QString filePath() const;
    void updateElidedName();
    void updatePreviewSize();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void loadPreviewPixmap();

private:
    QString m_filePath;
    QLabel *m_previewLabel      = nullptr;
    QLabel *m_nameLabel         = nullptr;
    QWidget *m_selectionOverlay = nullptr;
    QPixmap m_originalPixmap;
    bool m_selected = false;
};
