#include <QApplication>
#include <QString>
#include <iostream>

#include "../include/ConfigWindow.h"
#include "version.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    app.setApplicationName("KUB3 Configurator");
    app.setApplicationVersion(KUB3_VERSION_STR);

    ConfigWindow window(QString("/tmp/hardware.ini"), QString("/tmp/process.ini"));

    window.show();
    return app.exec();
}