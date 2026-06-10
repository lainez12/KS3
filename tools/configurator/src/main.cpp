#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFontDatabase>
#include <QMessageBox>
#include <QTextStream>
#include <exception>

#include "../include/ConfigWindow.h"
#include "version.h"

using namespace Kub3;

/**
 * Loads the touch-optimized QSS
 */
static void loadStyles(QApplication *app)
{
    // Load Configurator-specific QSS
    QFile f(":/styles/configurator.qss");

    if (f.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream ts(&f);
        app->setStyleSheet(ts.readAll());
        f.close();
    }
    else
    {
        qWarning() << "CRITICAL: Failed to load stylesheet ':/styles/configurator.qss'. UI may be unusable on touchscreen.";
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("KUB3 Configurator");
    app.setApplicationVersion(KUB3_VERSION_STR);

    // Initialize Style
    loadStyles(&app);

    qInfo() << "========================================";
    qInfo() << "  KUB3 CONFIGURATOR TOOL";
    qInfo() << "  Version:" << KUB3_VERSION_STR;
    qInfo() << "  Hardware Config Path:" << KUB3_HARDWARE_CONFIG_PATH;
    qInfo() << "  Process Config Path:" << KUB3_PROCESS_CONFIG_PATH;
    qInfo() << "  Admin Config Path:" << KUB3_ADMIN_CONFIG_PATH;
    qInfo() << "========================================";

    try
    {
        ConfigWindow window(KUB3_HARDWARE_CONFIG_PATH, KUB3_PROCESS_CONFIG_PATH, KUB3_ADMIN_CONFIG_PATH);

#if defined(BUILD_DEBUG)
        window.showMaximized();
#else
        window.showFullScreen();
#endif

        return app.exec();
    }
    catch (const std::exception &e)
    {
        qCritical() << "Fatal Configurator Error:" << e.what();
        QMessageBox::critical(nullptr, "Fatal Error",
                              QString("The configurator encountered an unhandled error:\n\n%1").arg(e.what()));
        return -1;
    }
    catch (...)
    {
        qCritical() << "Fatal Configurator Error: unknown type";
        QMessageBox::critical(nullptr, "Fatal Error", "An unknown fatal error occurred.");
        return -1;
    }
}
