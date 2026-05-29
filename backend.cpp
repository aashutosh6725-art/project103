#include "backend.h"
#include <QDebug>

Backend::Backend(QObject *parent) : QObject(parent)
{
    // Connect UdpTransport signal to our slot
    // When a packet arrives over Wi-Fi, onPacketReceived is called automatically
    connect(&m_transport, &UdpTransport::packetReceived,
            this, &Backend::onPacketReceived);

    // Start listening for incoming UDP packets immediately
    m_transport.startListening(45454);
}



void Backend::broadcastPresence()
{
    Packet p;
    p.type     = "presence";
    p.sender   = m_myNickname;
    p.receiver = "all";
    p.message  = m_myNickname;
    p.hopCount = 7;

    // Presence packets don't need encryption
    // Send raw — override encrypt for this case
    QByteArray iv, tag;
    m_transport.sendPacket(p);

    qDebug() << "[BACKEND] Broadcasted presence:" << m_myNickname;
}

// ─── Getters ────────────────────────────────────────────────────────────────

QStringList Backend::messages() const { return m_messages; }
QStringList Backend::peers()    const { return m_peers; }
QString Backend::myNickname()   const { return m_myNickname; }

// ─── Q_INVOKABLE functions ───────────────────────────────────────────────────

void Backend::setNickname(const QString &name)
{
    if (name.trimmed().isEmpty())
        return;

    m_myNickname = name.trimmed();
    m_myId       = "local";

    Peer me(m_myId, m_myNickname);
    m_manager.addPeer(me);

    emit myNicknameChanged();
    qDebug() << "[BACKEND] Local peer created:" << m_myId << "/" << m_myNickname;
    // Broadcast presence so others discover us
    broadcastPresence();
}

void Backend::sendMessage(const QString &toNickname, const QString &text)
{
    if (m_myNickname.isEmpty()) {
        qDebug() << "[BACKEND] Cannot send — nickname not set yet";
        return;
    }

    // Build packet directly — no peer lookup needed
    Packet p;
    p.id       = 0;
    p.sender   = m_myNickname;
    p.receiver = toNickname;
    p.message  = text;
    p.hopCount = 7;

    // Send over UDP immediately
    m_transport.sendPacket(p);

    // Show in local chat
    if (toNickname.toLower() == "all")
        appendMessage("You → All: " + text);
    else
        appendMessage("You → " + toNickname + ": " + text);

    qDebug() << "[BACKEND] Sending to:" << toNickname << "msg:" << text;
}

void Backend::checkInbox()
{
    Peer* me = m_manager.getPeer(m_myId);
    if (!me) return;

    QStringList incoming = me->drainInbox();
    for (const QString &line : incoming) {
        appendMessage(line);
    }
}

void Backend::addSimulatedPeer(const QString &peerId, const QString &nick)
{
    Peer newPeer(peerId, nick);
    m_manager.addPeer(newPeer);

    if (!m_peers.contains(peerId)) {
        m_peers.append(peerId);
        emit peersChanged();
    }

    qDebug() << "[BACKEND] Peer added:" << peerId << "/" << nick;
}

void Backend::connectPeers(const QString &idA, const QString &idB)
{
    Peer* a = m_manager.getPeer(idA);
    Peer* b = m_manager.getPeer(idB);

    if (!a || !b) {
        qDebug() << "[BACKEND] connectPeers failed";
        return;
    }

    a->addNeighbour(b);
    b->addNeighbour(a);

    qDebug() << "[BACKEND] Connected:" << idA << "<-->" << idB;
}

// ─── UDP incoming packet handler ─────────────────────────────────────────────
void Backend::onPacketReceived(const Packet &packet)
{
    qDebug() << "[BACKEND] UDP packet arrived from"
             << packet.sender << "to" << packet.receiver
             << "| type:" << packet.type;

    // Ignore packets we sent ourselves
    if (packet.sender == m_myNickname)
        return;

    // Handle presence packet — auto-discover peer
    if (packet.type == "presence") {
        if (!m_peers.contains(packet.sender)) {
            m_peers.append(packet.sender);
            emit peersChanged();
            qDebug() << "[BACKEND] Discovered peer:" << packet.sender;

            // Reply with our own presence so they discover us too
            broadcastPresence();
        }
        return;
    }

    // Auto-discover sender even for message packets
    if (!m_peers.contains(packet.sender)) {
        m_peers.append(packet.sender);
        emit peersChanged();
    }

    // Deliver if addressed to me directly
    if (packet.receiver == m_myNickname) {
        appendMessage(packet.sender + " → You: " + packet.message);
        return;
    }

    // Deliver broadcast ("all") messages — but not our own (already filtered above)
    if (packet.receiver.toLower() == "all") {
        appendMessage(packet.sender + " → All: " + packet.message);
        return;
    }

    qDebug() << "[BACKEND] Packet not for me — ignoring";
}
// ─── Private helper ──────────────────────────────────────────────────────────

void Backend::appendMessage(const QString &line)
{
    m_messages.append(line);
    emit messagesChanged();
}
