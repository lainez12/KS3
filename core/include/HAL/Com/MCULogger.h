#pragma once

#include <QByteArray>
#include <QFile>
#include <QObject>
#include <QString>

// Forward declaration
namespace Kub3::HAL
{
    class MCUDriver;
}

namespace Kub3::HAL::Com
{
    class MCULogger : public QObject
    {
        Q_OBJECT
    public:
        explicit MCULogger(const QString &mcuName, MCUDriver *driver, const QString &logDir, QObject *parent = nullptr);
        ~MCULogger();

    public slots:
        void onDataReceived(const QByteArray &data);
        void onDataSent(const QByteArray &data);
        void flush(); // Forces remaining RAM buffer to disk

    private:
        void appendToBuffer(const QString &direction, const QByteArray &data);
        void writeChunkToDisk();

        QString m_logDir;
        QFile m_file;
        QByteArray m_textBuffer;

        // Write to disk every 64KB of text to optimize NVMe I/O
        const int MAX_BUFFER_SIZE = 64 * 1024;
    };
}
