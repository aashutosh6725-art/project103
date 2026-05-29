#include "crypto.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <QDebug>

// 32 bytes = 256 bits shared key
// In production this would be exchanged via QR code
const QByteArray Crypto::SHARED_KEY = QByteArray::fromHex(
    "4d657368546c6b32303236536563726574"  // "MeshTlk2026Secret" in hex
    "000000000000000000000000000000"       // padded to 32 bytes
    );

QByteArray Crypto::encrypt(const QString &plainText,
                           QByteArray &iv,
                           QByteArray &tag)
{
    QByteArray plainBytes = plainText.toUtf8();

    // Generate random 12-byte IV
    iv.resize(12);
    if (RAND_bytes(reinterpret_cast<unsigned char*>(iv.data()), 12) != 1) {
        qDebug() << "[CRYPTO] Failed to generate IV";
        return QByteArray();
    }

    // Prepare output buffer — same size as input for GCM
    QByteArray cipherText(plainBytes.size(), 0);
    tag.resize(16);

    // Create and init the cipher context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qDebug() << "[CRYPTO] Failed to create cipher context";
        return QByteArray();
    }

    int len = 0;
    bool ok = true;

    // Init AES-256-GCM encryption
    ok &= EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1;

    // Set IV length to 12 bytes
    ok &= EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, nullptr) == 1;

    // Set key and IV
    ok &= EVP_EncryptInit_ex(ctx,
                             nullptr,
                             nullptr,
                             reinterpret_cast<const unsigned char*>(Crypto::SHARED_KEY.constData()),
                             reinterpret_cast<const unsigned char*>(iv.constData())) == 1;

    // Encrypt the message
    ok &= EVP_EncryptUpdate(ctx,
                            reinterpret_cast<unsigned char*>(cipherText.data()),
                            &len,
                            reinterpret_cast<const unsigned char*>(plainBytes.constData()),
                            plainBytes.size()) == 1;

    // Finalise encryption
    int finalLen = 0;
    ok &= EVP_EncryptFinal_ex(ctx,
                              reinterpret_cast<unsigned char*>(cipherText.data()) + len,
                              &finalLen) == 1;

    // Get the authentication tag
    ok &= EVP_CIPHER_CTX_ctrl(ctx,
                              EVP_CTRL_GCM_GET_TAG,
                              16,
                              tag.data()) == 1;

    EVP_CIPHER_CTX_free(ctx);

    if (!ok) {
        qDebug() << "[CRYPTO] Encryption failed";
        return QByteArray();
    }

    qDebug() << "[CRYPTO] Encrypted" << plainBytes.size() << "bytes";
    return cipherText;
}

QString Crypto::decrypt(const QByteArray &cipherText,
                        const QByteArray &iv,
                        const QByteArray &tag)
{
    QByteArray plainText(cipherText.size(), 0);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qDebug() << "[CRYPTO] Failed to create cipher context";
        return QString();
    }

    int len = 0;
    bool ok = true;

    // Init AES-256-GCM decryption
    ok &= EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1;

    // Set IV length
    ok &= EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, nullptr) == 1;

    // Set key and IV
    ok &= EVP_DecryptInit_ex(ctx,
                             nullptr,
                             nullptr,
                             reinterpret_cast<const unsigned char*>(Crypto::SHARED_KEY.constData()),
                             reinterpret_cast<const unsigned char*>(iv.constData())) == 1;

    // Decrypt
    ok &= EVP_DecryptUpdate(ctx,
                            reinterpret_cast<unsigned char*>(plainText.data()),
                            &len,
                            reinterpret_cast<const unsigned char*>(cipherText.constData()),
                            cipherText.size()) == 1;

    // Set the expected authentication tag
    ok &= EVP_CIPHER_CTX_ctrl(ctx,
                              EVP_CTRL_GCM_SET_TAG,
                              16,
                              const_cast<char*>(tag.constData())) == 1;

    // Finalise — this is where GCM verifies the tag
    // Returns 1 if tag matches, -1 if tampered
    int finalLen = 0;
    int finalResult = EVP_DecryptFinal_ex(ctx,
                                          reinterpret_cast<unsigned char*>(plainText.data()) + len,
                                          &finalLen);

    EVP_CIPHER_CTX_free(ctx);

    if (finalResult != 1) {
        qDebug() << "[CRYPTO] Decryption FAILED — message tampered or wrong key";
        return QString();  // empty string = reject this message
    }

    qDebug() << "[CRYPTO] Decrypted successfully";
    return QString::fromUtf8(plainText);
}