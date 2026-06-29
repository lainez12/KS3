#include <Views/Components/Colors.h>
#include <Views/Components/ScreenshotThumbnailWidget.h>

#include <QFile>
#include <QFileInfo>
#include <QFontMetrics>
#include <QFrame>
#include <QGridLayout>
#include <QImageReader>
#include <QLabel>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QVBoxLayout>

ScreenshotThumbnailWidget::ScreenshotThumbnailWidget(const QString &filePath, QWidget *parent) : QFrame(parent), m_filePath(filePath)
{
    setObjectName("screenshotThumbnailCard");
    setCursor(Qt::PointingHandCursor);
    setFrameShape(QFrame::NoFrame);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    m_previewLabel = new QLabel(this);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setMinimumHeight(140);
    m_previewLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_previewLabel->setStyleSheet("background: #0F1D2A; border-radius: 8px;");
    m_previewLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    layout->addWidget(m_previewLabel, 1);

    m_nameLabel = new QLabel(QFileInfo(filePath).fileName(), this);
    m_nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_nameLabel->setStyleSheet("color: #1F2D3D; font-size: 18px; font-weight: 600;");
    m_nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_nameLabel->setFixedHeight(26);
    m_nameLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    layout->addWidget(m_nameLabel, 0);

    m_selectionOverlay = new QWidget(this);
    m_selectionOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_selectionOverlay->setStyleSheet(QString("background-color: rgba(0, 188, 209, 110); border-radius: 12px;"));
    m_selectionOverlay->hide();
    m_selectionOverlay->raise();

    setStyleSheet(
        "QFrame#screenshotThumbnailCard {"
        "  background: white;"
        "  border: 2px solid transparent;"
        "  border-radius: 12px;"
        "}"
        "QFrame#screenshotThumbnailCard[selected=\"false\"]:hover {"
        "  border-color: " TURQUOISE_COLOR ";"
        "}"
        "QFrame#screenshotThumbnailCard[selected=\"true\"] {"
        "  border-color: " TURQUOISE_COLOR ";"
        "}");

    loadPreviewPixmap();
}

void ScreenshotThumbnailWidget::setSelected(bool selected)
{
    if (m_selected == selected)
    {
        return;
    }

    m_selected = selected;
    m_selectionOverlay->setVisible(selected);
    setProperty("selected", selected);
    update();
}

bool ScreenshotThumbnailWidget::isSelected() const
{
    return m_selected;
}

QString ScreenshotThumbnailWidget::filePath() const
{
    return m_filePath;
}

void ScreenshotThumbnailWidget::updateElidedName()
{
    const int availableWidth = qMax(0, m_nameLabel->width());
    QFontMetrics metrics(m_nameLabel->font());
    m_nameLabel->setText(metrics.elidedText(QFileInfo(m_filePath).fileName(), Qt::ElideRight, availableWidth));
    setToolTip(QFileInfo(m_filePath).fileName());
}

void ScreenshotThumbnailWidget::updatePreviewSize()
{
    if (m_originalPixmap.isNull())
    {
        return;
    }

    const QSize targetSize = m_previewLabel->size() * m_previewLabel->devicePixelRatioF();
    if (targetSize.isEmpty())
    {
        return;
    }

    const QPixmap scaled = m_originalPixmap.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    m_previewLabel->setPixmap(scaled);
}

void ScreenshotThumbnailWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        setSelected(!m_selected);
    }

    QFrame::mousePressEvent(event);
}

void ScreenshotThumbnailWidget::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    m_selectionOverlay->setGeometry(rect());
    m_selectionOverlay->raise();
    updatePreviewSize();
    updateElidedName();
}

void ScreenshotThumbnailWidget::loadPreviewPixmap()
{
    QImageReader reader(m_filePath);
    reader.setAutoTransform(true);

    const QImage image = reader.read();
    if (image.isNull())
    {
        m_previewLabel->setText("Preview unavailable");
        m_previewLabel->setStyleSheet("color: white; background: #0F1D2A; border-radius: 8px;");
        return;
    }

    m_originalPixmap = QPixmap::fromImage(image);
    updatePreviewSize();
}
