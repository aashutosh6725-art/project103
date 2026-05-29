#ifndef PACKETQUEUE_H
#define PACKETQUEUE_H


#include "packet.h"
#include <QQueue>

class PacketQueue
{
private:
    QQueue<Packet> queue;

public:
    void addPacket(const Packet &packet);
    bool processPacket(Packet &outPacket);
};


#endif // PACKETQUEUE_H
