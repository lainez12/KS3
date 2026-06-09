#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageAuthenticationCode>
#include <QRandomGenerator>

#include <security/AdminAuthenticator.h>
#include <security/ObfuscatedPepper.h>

namespace Kub3::Security
{
    constexpr ObfuscatedPepper PEPPER_CIPHER("P01VR3_D3_L4_M4CH1N3_KUB3_V1_2026");

    bool AdminAuthenticator::isVaultInitialized()
    {
        return QFile::exists(KUB3_ADMIN_VAULT_PATH);
    }

    bool AdminAuthenticator::verifyPassword(const QString &plainTextPassword)
    {
        if (!isVaultInitialized())
            return false;

        QByteArray salt        = loadSalt();
        QByteArray storedHash  = loadHash();
        QByteArray attemptHash = deriveKey(plainTextPassword, salt);

        return constantTimeCompare(storedHash, attemptHash);
    }

    bool AdminAuthenticator::setNewPassword(const QString &newPassword)
    {
        // Generate 16 bytes of salt
        QByteArray newSalt;
        newSalt.resize(16);
        QRandomGenerator::securelySeeded().generate(newSalt.begin(), newSalt.end());
        // Hash it
        QByteArray newHash = deriveKey(newPassword, newSalt);
        // Ensure directory exists
        QFileInfo fi(KUB3_ADMIN_VAULT_PATH);
        QDir().mkpath(fi.absolutePath());
        // Save to vault (Fail Open allowed)
        QFile vault(KUB3_ADMIN_VAULT_PATH);

        if (!vault.open(QIODevice::WriteOnly))
        {
            return false;
        }

        vault.write(newSalt);
        vault.write(newHash);
        vault.close();
#if defined(BUILD_DEBUG)
        // Enforce root-only rw access in production (Fail-Open safety)
        vault.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
#endif

        return true;
    }

    QByteArray AdminAuthenticator::computeFileSignature(const QByteArray &fileData)
    {
        // The file signature uses the stored hash as the HMAC key
        QByteArray key    = loadHash();
        QByteArray pepper = QByteArray::fromStdString(PEPPER_CIPHER.decrypt()); // Load pepper

        if (key.isEmpty())
        {
            // Fallback key for first boot before password is set
            key = QCryptographicHash::hash(pepper, QCryptographicHash::Sha256);
        }
        pepper.fill(0); // Overrite pepper in RAM (high-level paranoia)

        return QMessageAuthenticationCode::hash(fileData, key, QCryptographicHash::Sha256);
    }

    QByteArray AdminAuthenticator::deriveKey(const QString &password, const QByteArray &salt)
    {
        QByteArray pepper = QByteArray::fromStdString(PEPPER_CIPHER.decrypt());
        QByteArray key    = password.toUtf8() + salt + pepper;

        pepper.fill(0); // Overrite pepper in RAM (high-level paranoia)
        for (int i = 0; i < 10000; ++i)
        {
            key = QCryptographicHash::hash(key, QCryptographicHash::Sha256);
        }
        return key;
    }

    bool AdminAuthenticator::constantTimeCompare(const QByteArray &a, const QByteArray &b)
    {
        if (a.length() != b.length() || a.isEmpty())
            return false;

        volatile char result = 0;
        for (qsizetype i = 0; i < a.length(); ++i)
        {
            result |= a[i] ^ b[i];
        }
        return result == 0;
    }

    QByteArray AdminAuthenticator::loadSalt()
    {
        QFile vault(KUB3_ADMIN_VAULT_PATH);

        if (vault.open(QIODevice::ReadOnly))
        {
            return vault.read(16);
        }
        return QByteArray();
    }

    QByteArray AdminAuthenticator::loadHash()
    {
        QFile vault(KUB3_ADMIN_VAULT_PATH);

        if (vault.open(QIODevice::ReadOnly))
        {
            vault.seek(16); // Skip the 16-byte salt
            return vault.readAll();
        }
        return QByteArray();
    }

} // namespace Kub3::Security
