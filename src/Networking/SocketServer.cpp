#include "SocketServer.h"

#include "Common.h"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/SocketSelector.hpp"
#include "SFML/Network/TcpListener.hpp"
#include "SFML/Network/TcpSocket.hpp"
#include "SFML/System/Sleep.hpp"
#include "SFML/System/Time.hpp"
#include "logy.h"

#include <algorithm>
#include <cstddef>
#include <exception>
#include <mutex>
#include <thread>
#include <utility>

namespace Networking {
SocketServer::SocketServer(sf::IpAddress ip, uint16_t port)
        : m_Port(port), m_Ip(ip) {
    m_ClientConnectedCallback = [](ID_t) {};
    m_ClientDisconnectedCallback = [](ID_t) {};
}

SocketServer::~SocketServer() { stop(); }

void SocketServer::start() {
    m_ListenThread = std::thread(&SocketServer::listenThread, this);

    while (!m_ListenThreadRunning && !m_ListenThreadFailed) {
        sf::sleep(sf::milliseconds(100));
    }
}

void SocketServer::stop() {
    m_ListenThreadRunning = false;
    if (m_ListenThread.joinable()) {
        m_ListenThread.join();
    }

    m_ClientsMutex.lock();
    for (auto& [clientId, clientInfo]: m_Clients) {
        clientInfo.isRunning = false;
        clientInfo.socket->disconnect();
        if (clientInfo.thread.joinable()) {
            clientInfo.thread.join();
        }
    }
    m_Clients.clear();
    m_ClientsMutex.unlock();

    LOG_INFO("Stopped");
}

void SocketServer::listenThread() {
    sf::TcpListener listener;

    if (listener.listen(m_Port, m_Ip) != sf::Socket::Status::Done) {
        m_ListenThreadFailed = true;
        return;
    }

    m_ListenThreadRunning = true;

    listener.setBlocking(false);

    while (m_ListenThreadRunning) {
        sf::TcpSocket *newClient = new sf::TcpSocket;
        sf::Socket::Status status = listener.accept(*newClient);

        if (status > sf::Socket::Status::NotReady) {
            LOG_WARNING("Failed to accept client");
            delete newClient;
            m_ListenThreadRunning = false;
            break;
        } else if (status == sf::Socket::Status::Done) {
            ID_t newClientId = m_CurrentClientId++;

            m_ClientsMutex.lock();

            clientInfo *newClientInfo = &m_Clients[newClientId];
            newClientInfo->id = newClientId;
            newClientInfo->isRunning = true;
            newClientInfo->socket = newClient;
            newClientInfo->thread =
                    std::thread(&SocketServer::clientThread, this, newClientInfo);

            m_ClientsMutex.unlock();

            m_ConnectedClientsMutex.lock();
            m_ConnectedClients.push_back(newClientId);
            m_ConnectedClientsMutex.unlock();
        } else {
            delete newClient;
        }

        sf::sleep(sf::milliseconds(100));
    }

    listener.close();
}

void SocketServer::clientThread(clientInfo *clientInfo) {
    sf::SocketSelector selector;
    selector.add(*clientInfo->socket);

    sf::Packet idPacket;
    idPacket << ID_t_MAX << clientInfo->id;
    clientInfo->socket->send(idPacket);

    while (clientInfo->isRunning && clientInfo->socket->getRemotePort() != 0 &&
           isListenThreadRunning()) {
        if (selector.wait(sf::milliseconds(10))) {
            sf::Packet packet;
            if (clientInfo->socket->receive(packet) != sf::Socket::Status::Done) {
                LOG_WARNING("Failed to receive packet!");
                break;
            }

            m_ReceivedPacketsMutex.lock();
            m_ReceivedPackets[clientInfo->id].push_back(packet);
            m_ReceivedPacketsMutex.unlock();
        }
    }

    clientInfo->socket->disconnect();
    delete clientInfo->socket;

    clientInfo->isRunning = false;

    m_DisconnectedClientsMutex.lock();
    m_DisconnectedClients.push_back(clientInfo->id);
    m_DisconnectedClientsMutex.unlock();

    LOG_INFO("Finished client", clientInfo->id.load());
}

bool SocketServer::isListenThreadRunning() { return m_ListenThreadRunning; }

size_t SocketServer::clientsCount() {
    std::lock_guard guard(m_ClientsMutex);
    return m_Clients.size();
}

bool SocketServer::clientOnline(ID_t id) {
    std::lock_guard guard(m_ClientsMutex);
    return m_Clients.find(id) != m_Clients.end();
}

void SocketServer::send(ID_t id, sf::Packet packet) {
    std::lock_guard guard(m_ClientsMutex);
    if (m_Clients.find(id) == m_Clients.end()) {
        LOG_WARNING("Client not online:", id);
        return;
    }

    if (m_Clients.at(id).socket->send(packet) != sf::Socket::Status::Done) {
        LOG_WARNING("Failed to send packet to client", id);
    }
}

void SocketServer::sendAll(sf::Packet packet, ID_t exclude) {
    std::lock_guard guard(m_ClientsMutex);
    for (auto& [id, LOG_INFO]: m_Clients) {
        if (id == exclude)
            continue;

        if (LOG_INFO.socket->send(packet) != sf::Socket::Status::Done) {
            LOG_WARNING("Failed to send packet to client", id);
        }
    }
}

void SocketServer::setClientConnectedCallback(
        ClientConnectedCallback callback) {
    m_ClientConnectedCallback = std::move(callback);
}

void SocketServer::setClientDisconnectedCallback(
        ClientDisconnectedCallback callback) {
    m_ClientDisconnectedCallback = std::move(callback);
}

void SocketServer::handleCallbacks() {
    if (!isListenThreadRunning()) {
        LOG_INFO("Listen thread not running");
        return;
    }

    m_ClientsMutex.lock();
    std::erase_if(m_Clients, [](auto& LOG_INFO) {
        if (LOG_INFO.second.isRunning == false) {
            if (LOG_INFO.second.thread.joinable()) {
                LOG_INFO.second.thread.join();
            }
            return true;
        }
        return false;
    });
    m_ClientsMutex.unlock();

    m_ConnectedClientsMutex.lock();
    for (ID_t id: m_ConnectedClients) {
        m_ClientConnectedCallback(id);
    }
    m_ConnectedClients.clear();
    m_ConnectedClientsMutex.unlock();

    m_DisconnectedClientsMutex.lock();
    for (ID_t id: m_DisconnectedClients) {
        m_ClientDisconnectedCallback(id);
    }
    m_DisconnectedClients.clear();
    m_DisconnectedClientsMutex.unlock();

    m_ReceivedPacketsMutex.lock();
    for (auto& [senderId, packets]: m_ReceivedPackets) {
        for (sf::Packet& packet: packets) {
            ID_t packetType;

            try {
                packetType = getPacketType(packet);
            } catch (const std::exception& e) {
                LOG_WARNING("Unable to find packet type");
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

            m_Callbacks.at(packetType)(senderId, packet);
        }
    }
    m_ReceivedPackets.clear();
    m_ReceivedPacketsMutex.unlock();
}

void SocketServer::kickClient(ID_t id) {
    std::lock_guard guard(m_ClientsMutex);
    if (m_Clients.find(id) == m_Clients.end()) {
        LOG_WARNING("Tried to kick non-existent client:", id);
        return;
    }

    m_Clients.at(id).socket->disconnect();
}
}
