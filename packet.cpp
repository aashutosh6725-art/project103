#include "packet.h"

Packet::Packet() {
    id        = 0;
    sender    = "";
    receiver  = "";
    message   = "";
    type      = "message";
    hopCount  = 7;
    timestamp = QDateTime::currentDateTime();
    encrypted = false;
}