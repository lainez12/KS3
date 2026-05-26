#pragma once

#include <QColor>
#include <QHideEvent>
#include <QShowEvent>
#include <QString>
#include <QWidget>
#include <memory>

#include <ViewModels/IViewModel.h>
#include <Views/NavButtonManager.h>
#include <utils.h>

namespace Kub3::UI {
    enum class ViewId {
        HOME_VIEW,
        EXPOSURE_SETTINGS_VIEW,
        FAVORITE_EXPOSURE_SETTINGS_VIEW,
        SETTINGS_VIEW,
        SETTINGS_ABOUT_VIEW,
        ALIGNMENT_VIEW,
        MACHINE_STATUS_VIEW
    };
}

namespace Kub3::UI::Views {

    struct TitleBarConfig {
        QString viewTitle = "";
        QColor textColor;
        QColor bgColor;
        QString iconPath;
        QString sectionTitle;
        bool showTitleBar   = false;
        bool m_showLeftLogo = true;

        TitleBarConfig() = default;

        TitleBarConfig(const QString &t, const QColor &ct, const QColor &cbg, const QString &i, const QString &st, const bool s, const bool sl) : viewTitle(t), textColor(ct), bgColor(cbg), iconPath(i), sectionTitle(st), showTitleBar(s), m_showLeftLogo(sl) {
        }
    };

    /**
     * @class ViewBase
     * Classe de base pour toutes les vues de l'application
     *
     * Gère :
     * - La gestion du modèle de vue (ViewModel)
     * - Les propriétés d'affichage (titre, barres, logo)
     * - Les boutons de navigation via NavButtonManager
     * - Les signaux pour synchroniser avec MainWindow
     *
     */
    class ViewBase : public QWidget {
        Q_OBJECT

    public:
        explicit ViewBase(Unique<ViewModels::IViewModel> viewModel, QWidget *parent = nullptr);
        virtual ~ViewBase() = default;

        // --- Display Properties ---
        QString getViewTitle() const {
            return m_titleBar.viewTitle;
        }

        bool showTitleBar() const {
            return m_titleBar.showTitleBar;
        }

        bool showCentralLogo() const {
            return m_showCentralLogo;
        }
        const NavButtonManager &getNavButtonManager() const {
            return m_buttonManager;
        }

        /**
         * Ajoute un bouton de navigation
         * @param position Position du bouton ("left", "center", "right")
         * @param config Configuration du bouton
         * @param order Ordre d'apparition (optionnel)
         */
        void addNavButton(const QString &position, const NavButtonConfig &config, int order = -1) {
            m_buttonManager.addButton(position, config, order);
            emit s_buttonConfigsUpdated();
        }

        void removeNavButton(const QString &buttonId) {
            m_buttonManager.removeButton(buttonId);
            emit s_buttonConfigsUpdated();
        }

        void clearNavButtons() {
            m_buttonManager.clearButtons();
            emit s_buttonConfigsUpdated();
        }

        void setNavButtonEnabled(const QString &buttonId, bool enabled) {
            if (m_buttonManager.setButtonEnabled(buttonId, enabled)) {
                emit s_buttonStateChanged(buttonId, enabled);
            }
        }

        void setNavButtonVisible(const QString &buttonId, bool visible) {
            if (m_buttonManager.setButtonVisible(buttonId, visible)) {
                emit s_buttonStateChanged(buttonId, visible);
            }
        }

        void setNavButtonText(const QString &buttonId, const QString &text) {
            if (m_buttonManager.setButtonText(buttonId, text)) {
                emit s_buttonTextChanged(buttonId, text);
            }
        }

    signals:
        void s_openView(Kub3::UI::ViewId viewId);
        void s_goBack();
        void s_goBackHome();
        void s_buttonConfigsUpdated();

        void s_buttonStateChanged(const QString &buttonId, bool newState);
        void s_buttonTextChanged(const QString &buttonId, const QString &newText);

    public slots:

    public:
        TitleBarConfig m_titleBar;

    protected:
        void showEvent(QShowEvent *event) override;
        void hideEvent(QHideEvent *event) override;
        void setUpShawedBoxStyle(QWidget *widget);

    protected:
        Unique<ViewModels::IViewModel> m_viewModel;
        bool m_showCentralLogo;
        NavButtonManager m_buttonManager;

    private:
        virtual void createNavButtonsConfigs() = 0;
        virtual void configTitleBar()          = 0;
    };

}