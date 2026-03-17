#include <QApplication>

#include "Views/MainWindow.h"
#include "version.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow w;

    qDebug() << "Current App Version:" << KUB3_VERSION_STR;

    w.show();
    return app.exec();
}
