#pragma once

#include <QByteArray>
#include <QString>
#include <optional>

namespace Kub3::Security
{
    class AdminAuthenticator
    {
    public:
        // Returns true if the provided password matches the stored credentials
        static bool verifyPassword(const QString &plainTextPassword);
        // Generates a new salt and hash for a new password (used during initial setup)
        static bool setNewPassword(const QString &newPassword);
        // Computes an HMAC for the admin configuration file to prevent offline tampering
        static QByteArray computeFileSignature(const QByteArray &fileData);
        // Returns true if a vault exists
        static bool isVaultInitialized();

    private:
        static QByteArray deriveKey(const QString &password, const QByteArray &salt);
        static bool constantTimeCompare(const QByteArray &a, const QByteArray &b);

        static QByteArray loadSalt();
        static QByteArray loadHash();
    };

} // namespace Kub3::Security
