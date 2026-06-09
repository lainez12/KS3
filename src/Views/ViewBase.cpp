#include "Views/Components/Colors.h"
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QLineEdit>
#include <QSpinBox>
#include <Views/ViewBase.h>

namespace Kub3::UI::Views {

    ViewBase::ViewBase(Unique<ViewModels::IViewModel> viewModel, QWidget *parent) :
        QWidget(parent),
        m_viewModel(std::move(viewModel)) {
    }

    void ViewBase::showEvent(QShowEvent *event) {
        if (m_viewModel) {
            m_viewModel->loadConnections();
        }
        QWidget::showEvent(event);
    }

    void ViewBase::hideEvent(QHideEvent *event) {
        QWidget::hideEvent(event);
        if (m_viewModel) {
            m_viewModel->unloadConnections();
        }
    }

    void ViewBase::setUpShawedBoxStyle(QWidget *widget) {
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
        shadow->setBlurRadius(50);                   // Softness
        shadow->setOffset(5, 10);                    // Direction (x, y)
        shadow->setColor(QColor(BLUE_COLOR_SHADOW)); // Blue with transparency
        widget->setGraphicsEffect(shadow);
    }

    void ViewBase::showAnOverlayMessage(QString text) {
        m_messages->setText(text);
        m_messages->show();
    }

    void ViewBase::simulationKey(Qt::Key keyCode, const QString &text) {
        QWidget *focusedWidget = QApplication::focusWidget();
        if (focusedWidget) {
            QKeyEvent *keyPress = new QKeyEvent(QEvent::KeyPress, keyCode, Qt::NoModifier, text);
            QApplication::postEvent(focusedWidget, keyPress);

            QKeyEvent *keyRelease = new QKeyEvent(QEvent::KeyRelease, keyCode, Qt::NoModifier, text);
            QApplication::postEvent(focusedWidget, keyRelease);
        }
    }

    void ViewBase::clearInputSelected() {
        QWidget *focusedWidget = QApplication::focusWidget();
        if (!focusedWidget) {
            return;
        }
        if (QSpinBox *spinBox = qobject_cast<QSpinBox *>(focusedWidget)) {
            spinBox->clear();
        } else if (QDoubleSpinBox *doubleSpinBox = qobject_cast<QDoubleSpinBox *>(focusedWidget)) {
            doubleSpinBox->clear();
        } else if (QLineEdit *lineEdit = qobject_cast<QLineEdit *>(focusedWidget)) {
            lineEdit->clear();
        }
    }

} // namespace Kub3::UI::Views
