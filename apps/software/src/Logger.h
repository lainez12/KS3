#pragma once

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QTextStream>

#include <iostream>

class Logger
{
public:
    static void init()
    {
        QMutexLocker locker(&mutex);
        if (file.isOpen())
            return;

        const QString logsDirPath = KUB3_LOGS_PATH;
        QDir dir(logsDirPath);

        if (!dir.exists())
        {
            dir.mkpath(".");
        }

        cleanupOldLogs(logsDirPath); // Run cleanup before creating the new log file

        QString fileName = logsDirPath + "/kub3_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + ".log";
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
    static void cleanupOldLogs(const QString &logDir)
    {
        QDir dir(logDir);
        // Only target log files to prevent accidental deletion of other config/bin files
        dir.setNameFilters(QStringList() << "*.log");

        // Fetch files sorted by Time (Newest First)
        QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::NoSymLinks, QDir::Time);

        QMap<QString, int> categoryCounts;
        QDateTime threshold = QDateTime::currentDateTime().addDays(-3); // 3 days ago

        for (const QFileInfo &fileInfo : list)
        {
            QString fileName = fileInfo.fileName();
            // Extract the prefix (e.g., "kub3", "Arduino1", "Arduino2")
            QString prefix = fileName.section('_', 0, 0);

            int count = categoryCounts.value(prefix, 0);

            // If we already kept 3 files for this prefix AND it's older than 3 days -> delete
            if (count >= 3 && fileInfo.lastModified() < threshold)
            {
                QFile::remove(fileInfo.absoluteFilePath());
                std::cout << "[Logger] Cleaned up old log file:" << fileName.toStdString() << std::endl;
            }
            else
            {
                // Increment counter for this prefix (sparing the file)
                categoryCounts[prefix] = count + 1;
            }
        }
    }

private:
    static inline QFile file;
    static inline QTextStream stream;
    static inline QMutex mutex;
};
