// Libs
#include <QApplication>
#include <QFontDatabase>
#include <QObject>
#include <QThread>

#include "ApplicationBuilder.h"
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
        qDebug() << "Loaded font family w/ name:" << familyName;
    }
    else
        qDebug() << "Failed to load font: ':/fonts/ArtNormFont.ttf'";
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

    return appBuilder
        .loadConfigurations("/tmp/hardware.ini", "/tmp/process.ini")
        .buildHardwareTier()
        .buildLogicTier()
        .buildUserInterfaceTier()
        .wireArchitecture()
        .run(app);
}
