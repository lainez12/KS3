#include <QApplication>
#include <QFontDatabase>
#include <QMessageBox>
#include <QObject>
#include <QThread>
#include <exception>

#include <ApplicationBuilder.h>

#include "Logger.h"

using namespace Kub3;

static QtMessageHandler originalHandler = nullptr;

static void qtLogsHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Logger::write(qFormatLogMessage(type, context, msg));

    if (originalHandler)
        originalHandler(type, context, msg);
}

static void loadStyles(QApplication *app)
{
    // Load QSS files
    QFile f(":/styles/app.qss");

    if (f.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream ts(&f);
        app->setStyleSheet(ts.readAll());
        f.close();
    }

    // Load fonts
    int fontId = QFontDatabase::addApplicationFont(":/fonts/ArtNormFont.ttf");
    if (fontId != -1)
    {
        QString familyName = QFontDatabase::applicationFontFamilies(fontId).at(0);
        qInfo() << "Loaded font family w/ name:" << familyName;
    }
    else
        qWarning() << "Failed to load font: ':/fonts/ArtNormFont.ttf'";
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Logger::init();                                           // Initilize logger (sets up log file location)
    originalHandler = qInstallMessageHandler(&qtLogsHandler); // Install log handler

    loadStyles(&app); // Load QSS file

    qInfo() << "========================================";
    qInfo() << "  KUB3 Control Software";
    qInfo() << "  Version:" << KUB3_VERSION_STR;
    qInfo() << "  Model:" << KUB_MODEL_STR;
    qInfo() << "  Log path:" << Logger::file_location();
    qInfo() << "========================================";

    ApplicationBuilder appBuilder;
    int ret;

    try
    {
        ret = appBuilder
                  .loadConfigurations(KUB3_HARDWARE_CONFIG_PATH, KUB3_PROCESS_CONFIG_PATH, KUB3_ADMIN_CONFIG_PATH)
                  .buildHardwareTier()
                  .buildLogicTier()
                  .buildUserInterfaceTier()
                  .wireArchitecture()
                  .run(app);
    }
    catch (const std::exception &e)
    {
        qCritical() << "Fatal exception:" << e.what();
        QMessageBox::critical(nullptr, "Fatal Error", QString("An unhandled error occurred:\n%1").arg(e.what()));
        return -1;
    }
    catch (...)
    {
        qCritical() << "Fatal exception: unknown (non-std::exception type)";
        QMessageBox::critical(nullptr, "Fatal Error", "An unknown fatal error occurred. The application will now close.");
        return -1;
    }

    return ret;
}
