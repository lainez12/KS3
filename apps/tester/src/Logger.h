#pragma once

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QTextStream>

class Logger
{
public:
    static void init()
    {
        QMutexLocker locker(&mutex);
        if (file.isOpen())
            return;

        QString logDir;
#ifdef BUILD_DEBUG
        // Local to project: ./logs/
        logDir = QCoreApplication::applicationDirPath() + "/logs";
#else
        // System path: /var/log/kub3/
        logDir = "/var/log/kub3";
#endif

        QDir dir(logDir);
        if (!dir.exists())
        {
            dir.mkpath(".");
        }

        QString fileName = logDir + "/kub3tester_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + ".log";
        file.setFileName(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            stream.setDevice(&file);
        else
        {
            qFatal() << "Failed to open log file. Stopping process.";
            exit(1);
        }
    }

    static void write(const QString &message)
    {
        QMutexLocker locker(&mutex);
        if (stream.device())
        {
            stream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")
                   << " " << message << Qt::endl;
            stream.flush(); // Ensure data is written even if app crashes
        }
    }

    static std::string file_location(void)
    {
        if (file.exists())
            return file.filesystemFileName().string();
        return "Error reading location";
    }

private:
    static inline QFile file;
    static inline QTextStream stream;
    static inline QMutex mutex;
};
