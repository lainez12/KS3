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
        connect(m_viewModel.get(), &ViewModels::BaseViewModel::s_showOverlayMsg, this, &ViewBase::ps_showOverlayMsg);
        connect(m_viewModel.get(), &ViewModels::BaseViewModel::s_createPopUp, this, &ViewBase::ps_createPopUp);
        connect(m_viewModel.get(), &ViewModels::BaseViewModel::s_createPopUpWithText, this, &ViewBase::ps_createPopUpWithText);
        connect(m_viewModel.get(), &ViewModels::BaseViewModel::s_closePopUp, this, &ViewBase::ps_closePopUp);
        connect(m_viewModel.get(), &ViewModels::BaseViewModel::s_updatePopUpMessage, m_popUpMessage.get(), &PopUpMessage::setMessageText);
        connect(m_viewModel.get(), &ViewModels::BaseViewModel::s_appendPopUpMessage, m_popUpMessage.get(), &PopUpMessage::appendMessageText);
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

    void ViewBase::setUpShadowedBoxStyle(QWidget *widget)
    {
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
        shadow->setBlurRadius(50);                   // Softness
        shadow->setOffset(5, 10);                    // Direction (x, y)
        shadow->setColor(QColor(BLUE_COLOR_SHADOW)); // Blue with transparency
        widget->setGraphicsEffect(shadow);
    }

    void ViewBase::ps_showOverlayMsg(QString text)
    {
        m_messages->setText(text);
        m_messages->show();
    }

    void ViewBase::ps_createPopUp(const QString &title, const PopUpActions &actions)
    {
        ps_createPopUpWithText(title, actions, "");
    }

    void ViewBase::ps_createPopUpWithText(const QString &title, const PopUpActions &actions, const QString &initialText)
    {
        if (m_popUpMessage)
        {
            m_popUpMessage->setTitleText(title);
            m_popUpMessage->setMessageText(initialText);
            m_popUpMessage->setButtons(actions);
            m_popUpMessage->showMessage();
        }
    }

    void ViewBase::ps_closePopUp()
    {
        if (m_popUpMessage)
        {
            m_popUpMessage->hide();
        }
    }

    void ViewBase::resizeEventOverride(QResizeEvent *event)
    {
        resizeEvent(event);
    }

    // --- Display Properties ---
    QString ViewBase::getViewTitle() const
    {
        return m_titleBar.viewTitle;
    }

    bool ViewBase::showTitleBar() const
    {
        return m_titleBar.showTitleBar;
    }

    bool ViewBase::showCentralLogo() const
    {
        return m_showCentralLogo;
    }

    bool ViewBase::shadowedBoxStyle() const
    {
        return m_shadowedBoxStyle;
    }

    const NavButtonManager &ViewBase::getNavButtonManager() const
    {
        return m_buttonManager;
    }

    void ViewBase::addNavButton(const QString &position, const NavButtonConfig &config, int order)
    {
        m_buttonManager.addButton(position, config, order);
        emit s_buttonConfigsUpdated();
    }

    void ViewBase::removeNavButton(const QString &buttonId)
    {
        m_buttonManager.removeButton(buttonId);
        emit s_buttonConfigsUpdated();
    }

    void ViewBase::clearNavButtons()
    {
        m_buttonManager.clearButtons();
        emit s_buttonConfigsUpdated();
    }

    void ViewBase::setNavButtonEnabled(const QString &buttonId, bool enabled)
    {
        if (m_buttonManager.setButtonEnabled(buttonId, enabled))
        {
            emit s_buttonStateChanged(buttonId, enabled);
        }
    }

    void ViewBase::switchColorNavButton(const QString &buttonId, bool enabled)
    {
        NavButtonConfig *button = m_buttonManager.getButton(buttonId);
        if (button)
        {
            emit s_switchColorButton(buttonId, enabled);
        }
    }

    void ViewBase::setNavButtonVisible(const QString &buttonId, bool visible)
    {
        if (m_buttonManager.setButtonVisible(buttonId, visible))
        {
            emit s_buttonStateChanged(buttonId, visible);
        }
    }

    void ViewBase::setNavButtonText(const QString &buttonId, const QString &text)
    {
        if (m_buttonManager.setButtonText(buttonId, text))
        {
            emit s_buttonTextChanged(buttonId, text);
        }
    }

    void ViewBase::setTitleBar(const TitleBarConfig &titleBar)
    {
        m_titleBar = titleBar;
    }

    const TitleBarConfig &ViewBase::getTitleBar(void)
    {
        return m_titleBar;
    }

    void ViewBase::clearWidget(QWidget *widget)
    {
        if (!widget)
            return;

        // Find all direct child widgets
        QList<QWidget *> children = widget->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly);

        for (QWidget *child : children)
        {
            child->setParent(nullptr); // Detach from parent
            child->deleteLater();      // Schedule safe deletion
        }
    }

} // namespace Kub3::UI::Views
