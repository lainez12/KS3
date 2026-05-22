#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QVBoxLayout>

#include "Views/MainWindow.h"
#include "ui_MainWindow.h"

#define TITLE_BAR "font-size: 40px; font-weight: bold;"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
    m_backgroundPixmap = QPixmap(":/images/bg.png");
    m_topBar           = ui->upBar;
    m_topBar->hide();

    QHBoxLayout *mainLayout = qobject_cast<QHBoxLayout *>(ui->bottomBar->layout());
    if (mainLayout) {
        m_bottomBarLeft   = new QHBoxLayout();
        m_bottomBarCenter = new QHBoxLayout();
        m_bottomBarRight  = new QHBoxLayout();

        mainLayout->addLayout(m_bottomBarLeft);
        mainLayout->addStretch();
        mainLayout->addLayout(m_bottomBarCenter);
        mainLayout->addStretch();
        mainLayout->addLayout(m_bottomBarRight);
    }

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(50);                  // Softness
    shadow->setOffset(0, 3);                    // Direction (x, y)
    shadow->setColor(QColor(0, 112, 219, 160)); // Blue with transparency
    ui->bottomBar->setGraphicsEffect(shadow);

    this->setAttribute(Qt::WA_OpaquePaintEvent, false);
}

MainWindow::~MainWindow() {
    // Déconnecter tous les signaux avant destruction
    for (auto &entry : m_views) {
        disconnectViewSignals(entry.second);
    }
    delete ui;
}

void MainWindow::addView(Kub3::UI::ViewId viewId, Kub3::UI::Views::ViewBase *view) {
    if (!view)
        return;
    ui->stackedWidget->addWidget(view);
    m_views.insert({viewId, view});

    // Établir les connexions de signaux
    connectViewSignals(view);
}

void MainWindow::connectViewSignals(Kub3::UI::Views::ViewBase *view) {
    connect(view, &Kub3::UI::Views::ViewBase::s_openView, this, &MainWindow::ps_openView);

    connect(view, &Kub3::UI::Views::ViewBase::s_goBack, this,
            [this]() {
                if (ui->stackedWidget->count() > 0) {
                    ui->stackedWidget->setCurrentIndex(std::max(0, ui->stackedWidget->currentIndex() - 1));
                }
            });

    connect(view, &Kub3::UI::Views::ViewBase::s_goBackHome, this, [this]() { ps_openView(Kub3::UI::ViewId::HOME_VIEW); });

    connect(view, &Kub3::UI::Views::ViewBase::s_buttonConfigsUpdated, this, &MainWindow::onViewButtonConfigsUpdated);

    connect(view, &Kub3::UI::Views::ViewBase::s_buttonStateChanged, this, &MainWindow::onViewButtonStateChanged);

    connect(view, &Kub3::UI::Views::ViewBase::s_buttonTextChanged, this, &MainWindow::onViewButtonTextChanged);
}

void MainWindow::disconnectViewSignals(Kub3::UI::Views::ViewBase *view) {
    disconnect(view, nullptr, this, nullptr);
}

void MainWindow::ps_openView(Kub3::UI::ViewId viewId) {
    auto it = m_views.find(viewId);
    if (it == m_views.end() || !it->second)
        return;

    Kub3::UI::Views::ViewBase *view = it->second;

    updateTopBar(view);
    updateBottomBar(view);

    ui->stackedWidget->setCurrentWidget(view);
    m_currentView = view;
}

void MainWindow::updateTopBar(Kub3::UI::Views::ViewBase *view) {
    if (!view || !m_topBar)
        return;

    const auto &configTitleBar = view->m_titleBar;
    if (configTitleBar.showTitleBar) {
        m_topBar->setTitleBarConfig(configTitleBar);
        m_topBar->show();
    } else {
        m_topBar->hide();
    }
}

void MainWindow::updateBottomBar(Kub3::UI::Views::ViewBase *view) {
    if (!view)
        return;

    clearBottomBar();
    showLogoIfNeeded(view);

    const auto &buttonManager = view->getNavButtonManager();

    for (const QString &position : QStringList{"left", "center", "right"}) {
        auto buttons = buttonManager.getButtonsByPosition(position);
        for (const auto &config : buttons) {
            if (!config.visible)
                continue;

            NavButton *btn = createNavButton(config);
            if (!btn)
                continue;

            NavButtonEntry entry;
            entry.button                        = btn;
            entry.position                      = position;
            m_bottomBarButtons[config.buttonId] = entry;

            if (position == "left")
                m_bottomBarLeft->addWidget(btn);
            else if (position == "center")
                m_bottomBarCenter->addWidget(btn);
            else if (position == "right")
                m_bottomBarRight->addWidget(btn);
        }
    }
}

void MainWindow::showLogoIfNeeded(Kub3::UI::Views::ViewBase *view) {
    if (view->showCentralLogo()) {
        QLabel *logo = new QLabel();
        logo->setPixmap(QPixmap(":/icons/logoKloe.png").scaledToHeight(80));
        logo->setAlignment(Qt::AlignCenter);
        m_bottomBarCenter->addWidget(logo);
    }
}

void MainWindow::clearBottomBar() {
    // Supprimer tous les widgets des layouts
    auto clearLayout = [](QHBoxLayout *layout) {
        QLayoutItem *item;
        while ((item = layout->takeAt(0))) {
            if (item->widget())
                item->widget()->deleteLater();
            delete item;
        }
    };

    clearLayout(m_bottomBarLeft);
    clearLayout(m_bottomBarCenter);
    clearLayout(m_bottomBarRight);

    m_bottomBarButtons.clear();
}

NavButton *MainWindow::createNavButton(const Kub3::UI::Views::NavButtonConfig &config) {
    NavButton *btn = new NavButton();
    btn->setup(config.text, config.color, config.iconPath);
    btn->setEnabledNavButton(config.enabled);

    connect(btn, &NavButton::clicked,
            this, [this, config]() {
                if (config.callback) {
                    config.callback(config.buttonId);
                }
            });

    return btn;
}

void MainWindow::onViewButtonConfigsUpdated() {
    updateBottomBar(m_currentView);
}

void MainWindow::onViewButtonStateChanged(const QString &buttonId, bool newState) {
    auto it = m_bottomBarButtons.find(buttonId);
    if (it != m_bottomBarButtons.end() && it.value().button) {
        it.value().button->setEnabledNavButton(newState);
    }
}

void MainWindow::onViewButtonTextChanged(const QString &buttonId, const QString &newText) {
    auto it = m_bottomBarButtons.find(buttonId);
    if (it != m_bottomBarButtons.end() && it.value().button) {
        it.value().button->setText(newText);
    }
}

void MainWindow::ps_stateChanged(const QString &stateName) {
    // Mise à jour basée sur l'état de la machine
}

void MainWindow::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    int scaledWidth = this->width() * 0.9;                                                     // Calculate the scaled width for bg image
    QPixmap scaled  = m_backgroundPixmap.scaledToWidth(scaledWidth, Qt::SmoothTransformation); // Scale while keeping the aspect ratio
    // Draw at the very bottom right of the ENTIRE window
    int x = this->width() - scaled.width();
    int y = this->height() - scaled.height();

    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.fillRect(this->rect(), Qt::white); // Draw the base white background for the whole window
    painter.drawPixmap(x, y, scaled);          // Draw the background image scaled pixmap
}
