#ifndef UDPTRANSPORT_H
#define UDPTRANSPORT_H

#include <QObject>
#include <QUdpSocket>
#include <QJsonObject>
#include "packet.h"

class UdpTransport : public QObject
{
    Q_OBJECT

public:
    explicit UdpTransport(QObject *parent = nullptr);
    void startListening(quint16 port = 45454);
    void sendPacket(const Packet &packet);

signals:
    void packetReceived(const Packet &packet);

private slots:
    void onDataReceived();

private:
    QUdpSocket m_socket;
    QByteArray serializePacket(const Packet &packet, const QByteArray &cipherText);
    Packet deserializePacket(const QJsonObject &json);
};

#endif // UDPTRANSPORT_H