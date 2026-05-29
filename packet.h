#ifndef PACKET_H
#define PACKET_H

#include <QString>
#include <QDateTime>
#include <QByteArray>

class Packet
{
public:
    Packet();
    int id;
    QString sender;
    QString receiver;
    QString message;
    QString type;      // "message" or "presence"
    int hopCount;
    QDateTime timestamp;
    QByteArray iv;
    QByteArray tag;
    bool encrypted;
};

#endif // PACKET_H