#include "udptransport.h"
#include "crypto.h"
#include <QNetworkDatagram>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHostAddress>
#include <QDebug>

UdpTransport::UdpTransport(QObject *parent) : QObject(parent)
{
    connect(&m_socket, &QUdpSocket::readyRead,
            this, &UdpTransport::onDataReceived);

    bool ok = m_socket.bind(QHostAddress::AnyIPv4, 45454,
                            QUdpSocket::ShareAddress |
                                QUdpSocket::ReuseAddressHint);
    if (ok)
        qDebug() << "[UDP] Socket bound on port 45454";
    else
        qDebug() << "[UDP] Bind failed:" << m_socket.errorString();
}

void UdpTransport::startListening(quint16 port)
{
    Q_UNUSED(port)
    // binding now happens in constructor
}

void UdpTransport::sendPacket(const Packet &packet)
{
    // Encrypt the message before sending
    Packet encryptedPacket = packet;
    QByteArray iv, tag;
    QByteArray cipherText = Crypto::encrypt(packet.message, iv, tag);

    if (cipherText.isEmpty()) {
        qDebug() << "[UDP] Encryption failed — aborting send";
        return;
    }

    encryptedPacket.iv        = iv;
    encryptedPacket.tag       = tag;
    encryptedPacket.encrypted = true;

    QByteArray data = serializePacket(encryptedPacket, cipherText);

    qint64 r1 = m_socket.writeDatagram(data, QHostAddress::LocalHost, 45454);
    qint64 r2 = m_socket.writeDatagram(data, QHostAddress::Broadcast, 45454);

    qDebug() << "[UDP] Sent packet" << packet.id
             << "localhost:" << r1
             << "broadcast:" << r2;
}

void UdpTransport::onDataReceived()
{
    while (m_socket.hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_socket.receiveDatagram();
        QByteArray data = datagram.data();

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "[UDP] JSON parse error:" << parseError.errorString();
            continue;
        }

        if (!doc.isObject()) continue;

        QJsonObject obj = doc.object();

        // Decrypt the message
        QByteArray cipherText = QByteArray::fromBase64(obj["cipherText"].toString().toUtf8());
        QByteArray iv         = QByteArray::fromBase64(obj["iv"].toString().toUtf8());
        QByteArray tag        = QByteArray::fromBase64(obj["tag"].toString().toUtf8());

        QString decrypted = Crypto::decrypt(cipherText, iv, tag);

        if (decrypted.isEmpty()) {
            qDebug() << "[UDP] Dropped packet — decryption failed";
            continue;
        }

        Packet packet;
        packet.id        = obj["id"].toInt();
        packet.sender    = obj["sender"].toString();
        packet.receiver  = obj["receiver"].toString();
        packet.message   = decrypted;   // plain text after decryption
        packet.hopCount  = obj["hopCount"].toInt();
        packet.type = obj["type"].toString("message");
        packet.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
        packet.encrypted = true;

        qDebug() << "[UDP] Received and decrypted packet from"
                 << packet.sender << "to" << packet.receiver;

        emit packetReceived(packet);
    }
}

QByteArray UdpTransport::serializePacket(const Packet &packet, const QByteArray &cipherText)
{
    QJsonObject obj;
    obj["id"]         = packet.id;
    obj["sender"]     = packet.sender;
    obj["receiver"]   = packet.receiver;
    obj["hopCount"]   = packet.hopCount;
    obj["type"]=packet.type;
    obj["timestamp"]  = packet.timestamp.toString(Qt::ISODate);

    // Store encrypted content as Base64 strings — safe for JSON
    obj["cipherText"] = QString::fromUtf8(cipherText.toBase64());
    obj["iv"]         = QString::fromUtf8(packet.iv.toBase64());
    obj["tag"]        = QString::fromUtf8(packet.tag.toBase64());

    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

Packet UdpTransport::deserializePacket(const QJsonObject &json)
{
    Q_UNUSED(json)
    return Packet(); // not used anymore — deserialization happens inline in onDataReceived
}