#include "packetqueue.h"

void PacketQueue::addPacket(const Packet &packet)
{
    queue.enqueue(packet);
}


bool PacketQueue::processPacket(Packet &outPacket)
{
    if (queue.isEmpty())
        return false;

    outPacket = queue.dequeue();
    return true;
}

