#include <QGraphicsDropShadowEffect>
#include <QPainter>

#include "Views/MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Main window
    {
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();

        m_backgroundPixmap = QPixmap(":/images/bg.png");

        ui->settingsBtn->setup("Settings", QColor("#0072BA"), ":/icons/settings.svg");
        ui->drawerBtn->setup("Open/Close", QColor("#0072BA"), ":/icons/eject.svg");

        shadow->setBlurRadius(50);                  // Softness
        shadow->setOffset(0, 3);                    // Direction (x, y)
        shadow->setColor(QColor(0, 112, 219, 160)); // Blue with transparency
        ui->bottomBar->setGraphicsEffect(shadow);

        this->setAttribute(Qt::WA_OpaquePaintEvent, false);
    }

    // Debug View
    // {
    //     m_debugView      = new DebugView(this);
    //     m_debugViewIndex = ui->stackedWidget->addWidget(m_debugView);

    //     connect(m_debugView, &DebugView::s_openMachineStatus, this, &MainWindow::openMachineStatusView);
    //     connect(m_debugView, &DebugView::s_initializationRequest, this, &MainWindow::s_initializationRequest);
    // }

    // {
    //     m_homeView      = new HomeView(this);
    //     m_homeViewIndex = ui->stackedWidget->addWidget(m_homeView);
    // }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addView(Kub3::UI::ViewId viewId, QWidget *view)
{
    if (!view)
        return;
    ui->stackedWidget->addWidget(view);
    m_views.insert({viewId, view});
}

void MainWindow::ps_openView(Kub3::UI::ViewId viewId)
{
    if (auto it = m_views.find(viewId); it != m_views.end() && it->second)
        ui->stackedWidget->setCurrentWidget(it->second);
    else
        qWarning() << "[MainWindow::ps_openView] Failed to find view.";
}

void MainWindow::ps_stateChanged(const QString &stateName)
{
    // m_debugView->updateMachineState(stateName);
}

void MainWindow::paintEvent(QPaintEvent *event)
{
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
