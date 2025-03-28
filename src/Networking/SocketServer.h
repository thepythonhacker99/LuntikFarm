#pragma once

#include "Common.h"
#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/TcpSocket.hpp"
#include "logy.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

namespace Networking {
using ClientConnectedCallback = std::function<void(ID_t)>;
using ClientDisconnectedCallback = std::function<void(ID_t)>;

using ServerReceiveInternalCallback = std::function<void(ID_t, sf::Packet)>;

struct clientInfo {
    std::atomic<ID_t> id;

    std::thread thread;

    std::atomic<bool> isRunning;
    sf::TcpSocket *socket;
};

class SocketServer {
public:
    SocketServer(sf::IpAddress ip, uint16_t port);
    ~SocketServer();

    bool isListenThreadRunning();
    size_t clientsCount();

    bool clientOnline(ID_t id);

    void start();
    void stop();

    void send(ID_t id, sf::Packet packet);
    void sendAll(sf::Packet packet, ID_t exclude = ID_t_MAX);

    void setClientConnectedCallback(ClientConnectedCallback callback);
    void setClientDisconnectedCallback(ClientDisconnectedCallback callback);

    void handleCallbacks();

    template<typename... args_t>
    void addReceiveCallback(ID_t id,
                            std::function<void(ID_t, args_t...)> callback) {
        if (!isPacketRegistered(id)) {
            LOG_WARNING("[Server] Packet with id", id, "hasn't beed registered yet");
            return;
        }

        if (!isPacketArgsValid<args_t...>(id)) {
            LOG_WARNING("[Server] Args of callback function are not valid", id);
            return;
        }

        if (m_Callbacks.find(id) != m_Callbacks.end()) {
            LOG_WARNING("[Server] Overriding callback with id", id);
        }

        m_Callbacks[id] = [callback](ID_t senderId, sf::Packet packet) {
            callback(senderId, packetReader<args_t>(packet)...);
        };
    }

    void kickClient(ID_t id);

private:
    void listenThread();
    void clientThread(clientInfo *clientInfo);

    std::atomic<bool> m_ListenThreadRunning = false;
    std::atomic<bool> m_ListenThreadFailed = false;
    std::thread m_ListenThread;

    std::mutex m_ClientsMutex;
    std::unordered_map<ID_t, clientInfo> m_Clients;

    ClientConnectedCallback m_ClientConnectedCallback;
    std::vector<ID_t> m_ConnectedClients;
    std::mutex m_ConnectedClientsMutex;

    ClientDisconnectedCallback m_ClientDisconnectedCallback;
    std::vector<ID_t> m_DisconnectedClients;
    std::mutex m_DisconnectedClientsMutex;

    std::unordered_map<ID_t, ServerReceiveInternalCallback> m_Callbacks;

    std::mutex m_ReceivedPacketsMutex;
    std::unordered_map<ID_t, std::vector<sf::Packet>> m_ReceivedPackets;

    ID_t m_CurrentClientId = 0;

    sf::IpAddress m_Ip;
    uint16_t m_Port;
};
}
