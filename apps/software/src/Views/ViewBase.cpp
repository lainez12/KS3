#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QLineEdit>
#include <QSpinBox>
#include <Views/Components/Colors.h>
#include <Views/ViewBase.h>

namespace Kub3::UI::Views
{

    ViewBase::ViewBase(Unique<ViewModels::BaseViewModel> viewModel, QWidget *parent) :
        QWidget(parent),
        m_viewModel(std::move(viewModel))
    {
    }

    void ViewBase::showEvent(QShowEvent *event)
    {
        if (m_viewModel)
        {
            m_viewModel->loadConnections();
        }
        QWidget::showEvent(event);
    }

    void ViewBase::hideEvent(QHideEvent *event)
    {
        QWidget::hideEvent(event);
        if (m_viewModel)
        {
            m_viewModel->unloadConnections();
        }
    }

    void ViewBase::setUpShawedBoxStyle(QWidget *widget)
    {
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
        shadow->setBlurRadius(50);                   // Softness
        shadow->setOffset(5, 10);                    // Direction (x, y)
        shadow->setColor(QColor(BLUE_COLOR_SHADOW)); // Blue with transparency
        widget->setGraphicsEffect(shadow);
    }

    void ViewBase::showAnOverlayMessage(QString text)
    {
        m_messages->setText(text);
        m_messages->show();
    }

    void ViewBase::showPopUpMessage(const QString &title, const QVector<PopUpMessage::ButtonConfig> &buttons)
    {
        showPopUpMessage(title, QString(), buttons);
    }

    void ViewBase::showPopUpMessage(const QString &title, const QString &message, const QVector<PopUpMessage::ButtonConfig> &buttons)
    {
        m_PopUpMessage->setTitleText(title);
        m_PopUpMessage->setMessageText(message);
        m_PopUpMessage->setButtons(buttons);
        m_PopUpMessage->showMessage();
    }

} // namespace Kub3::UI::Views
