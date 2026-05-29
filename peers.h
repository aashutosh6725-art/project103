#ifndef PEERS_H
#define PEERS_H

#include<QString>
#include<QList>
#include "packetqueue.h"
#include <QSet>
#include <QStringList>
class Peer {
private:
    QString id;
    QString nickname;
    PacketQueue inbox;
    QList<Peer*> neighbours;
    QSet<int> seenPackets;
    static int nextPacketId;    // ← add this

public:
    Peer(const QString &peerId, const QString &nickname = "");
    QString getId() const;
    QString getNickname() const;
    QStringList drainInbox();
    void addNeighbour(Peer* peer);
    void sendPacket(Peer &receiver, const QString &message);
    void receive();
    void forward(Packet p);
};

#endif // PEERS_H
