#include "peermanager.h"
#include <QDebug>

PeerManager::PeerManager() {
}

void PeerManager::addPeer(Peer& peer) {
    peers.insert(peer.getId(), peer);
    qDebug() << "peer added:" << peer.getId();
}

void PeerManager::removePeer(QString id) {
    peers.remove(id);
    qDebug() << "peer removed:" << id;
}

Peer* PeerManager::getPeer(QString id) {
    auto it = peers.find(id);
    if (it != peers.end()) {
        return &it.value();
    }
    return nullptr;
}

void PeerManager::printPeers() {
    qDebug() << "---- peers list ----";
    for (auto& peer : peers) {
        qDebug() << peer.getId();
    }
}