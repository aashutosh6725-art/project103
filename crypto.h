#ifndef CRYPTO_H
#define CRYPTO_H

#include <QByteArray>
#include <QString>

class Crypto
{
public:
    // The shared key — everyone in the mesh uses this same key
    // In a real app this would be exchanged via QR code or verbally
    // 32 bytes = 256 bits = AES-256
    static const QByteArray SHARED_KEY;

    // Encrypts plain text message
    // Returns encrypted bytes, and fills iv and tag
    static QByteArray encrypt(const QString &plainText,
                              QByteArray &iv,
                              QByteArray &tag);

    // Decrypts encrypted bytes using iv and tag
    // Returns plain text message, or empty string if decryption fails
    static QString decrypt(const QByteArray &cipherText,
                           const QByteArray &iv,
                           const QByteArray &tag);
};

#endif // CRYPTO_H