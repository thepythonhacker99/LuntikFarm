#include "SocketClient.h"

#include "Common.h"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/SocketSelector.hpp"
#include "SFML/Network/TcpListener.hpp"
#include "SFML/System/Time.hpp"
#include "logy.h"

#include <exception>
#include <thread>

namespace Networking {
SocketClient::SocketClient(sf::IpAddress ip, uint16_t port)
        : m_Port(port), m_Ip(ip) {}

SocketClient::~SocketClient() { stop(); }

void SocketClient::start() {
    if (m_Socket.connect(m_Ip, m_Port) != sf::Socket::Status::Done) {
        LOG_WARNING("Failed to connect to the server");
        return;
    }
    LOG_INFO("Connected to the server");
    m_ClientThreadRunning = true;
    m_ClientThread = std::thread(&SocketClient::clientThread, this);
}

void SocketClient::stop() {
    m_ClientThreadRunning = false;
    if (m_ClientThread.joinable()) {
        m_ClientThread.join();
    }

    LOG_INFO("Stopped");
}

void SocketClient::clientThread() {
    sf::SocketSelector selector;
    selector.add(m_Socket);

    while (isRunning() && m_Socket.getRemotePort() != 0) {
        if (selector.wait(sf::milliseconds(10))) {
            sf::Packet packet;
            if (m_Socket.receive(packet) != sf::Socket::Status::Done) {
                LOG_WARNING("Failed to receive packet!");
                break;
            }

            m_ReceivedPacketsMutex.lock();
            m_ReceivedPackets.push_back(packet);
            m_ReceivedPacketsMutex.unlock();
        }
    }

    m_Socket.disconnect();
    m_ClientThreadRunning = false;

    LOG_INFO("Finished client thread");
}

bool SocketClient::isRunning() { return m_ClientThreadRunning; }

void SocketClient::setDisconnectionCallback(DisconnectionCallback callback) {
    m_DisconnectionCallback = callback;
}

void SocketClient::send(sf::Packet packet) {
    if (m_Socket.send(packet) != sf::Socket::Status::Done) {
        LOG_WARNING("Failed to send packet");
    }
}

void SocketClient::handleCallbacks() {
    if (!m_ClientThreadRunning) {
        m_DisconnectionCallback();
    }

    m_ReceivedPacketsMutex.lock();
    for (sf::Packet& packet: m_ReceivedPackets) {
        ID_t packetType;

        try {
            packetType = getPacketType(packet);
        } catch (const std::exception& e) {
            LOG_WARNING("Unable to find packet type");
            continue;
        }

        if (packetType == ID_t_MAX) {
            packet >> m_ClientID;
            continue;
        }

        if (!isPacketRegistered(packetType)) {
            LOG_WARNING("Packet not registered", packetType);
            continue;
        }

        if (m_Callbacks.find(packetType) == m_Callbacks.end()) {
            LOG_WARNING("Received packet but no callback was assigned",
                        packetType);
            continue;
        }

        m_Callbacks.at(packetType)(packet);
    }
    m_ReceivedPackets.clear();
    m_ReceivedPacketsMutex.unlock();
}
}
