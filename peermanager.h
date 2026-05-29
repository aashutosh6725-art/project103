#ifndef PEERMANAGER_H
#define PEERMANAGER_H

#include <QMap>
#include <QString>
#include "peers.h"

class PeerManager
{
private:
    QMap<QString, Peer> peers;  // stores by value — simple and safe

public:
    PeerManager();
    void addPeer(Peer& peer);
    void removePeer(QString id);
    Peer* getPeer(QString id);
    void printPeers();
};

#endif // PEERMANAGER_H