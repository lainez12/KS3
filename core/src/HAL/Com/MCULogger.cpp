#include <QDateTime>
#include <QDebug>
#include <QDir>

#include <HAL/Com/MCULogger.h>
#include <HAL/MCUDriver.h>

namespace Kub3::HAL::Com
{
    MCULogger::MCULogger(const QString &mcuName, MCUDriver *driver, QObject *parent) :
        QObject(parent)
    {
        const QString logsDirPath = KUB3_LOGS_PATH;
        QDir dir(logsDirPath);

        if (!dir.exists())
            dir.mkpath(".");

        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
        m_file.setFileName(QString("%1/%2_%3.log").arg(logsDirPath, mcuName, timestamp));

        m_textBuffer.reserve(MAX_BUFFER_SIZE + 1024); // Pre-allocate to prevent reallocation hit

        // Self-wire to the driver
        if (driver)
        {
            connect(driver, &MCUDriver::s_parsedRxData, this, &MCULogger::onDataReceived);
            connect(driver, &MCUDriver::s_rawDataSent, this, &MCULogger::onDataSent);
        }
    }

    MCULogger::~MCULogger()
    {
        flush();
    }

    void MCULogger::onDataReceived(const QByteArray &data)
    {
        appendToBuffer("RX", data);
    }

    void MCULogger::onDataSent(const QByteArray &data)
    {
        appendToBuffer("TX", data);
    }

    void MCULogger::appendToBuffer(const QString &direction, const QByteArray &data)
    {
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        QString hexString = QString(data.toHex(' ').toUpper());

        QString line = QString("[%1] %2: %3\n").arg(timestamp, direction, hexString);

        m_textBuffer.append(line.toUtf8());

        if (m_textBuffer.size() >= MAX_BUFFER_SIZE)
        {
            writeChunkToDisk();
        }
    }

    void MCULogger::writeChunkToDisk()
    {
        if (m_textBuffer.isEmpty())
            return;

        if (!m_file.isOpen())
        {
            m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
        }

        if (m_file.isOpen())
        {
            m_file.write(m_textBuffer);
            m_file.flush();
            m_textBuffer.clear();
        }
        else
        {
            qWarning() << "[MCULogger] Failed to open log file:" << m_file.fileName();
        }
    }

    void MCULogger::flush()
    {
        writeChunkToDisk();
        if (m_file.isOpen())
        {
            m_file.close();
        }
    }
}
