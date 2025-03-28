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
using DisconnectionCallback = std::function<void()>;
using ClientReceiveInternalCallback = std::function<void(sf::Packet)>;

class SocketClient {
public:
    SocketClient(sf::IpAddress ip, uint16_t port);
    ~SocketClient();

    bool isRunning();

    void start();
    void stop();

    void send(sf::Packet packet);

    void handleCallbacks();

    void setDisconnectionCallback(DisconnectionCallback callback);

    template<typename... args_t>
    void addReceiveCallback(ID_t id, std::function<void(args_t...)> callback) {
        if (!isPacketRegistered(id)) {
            LOG_WARNING("[Client] Packet with id", id, " hasn't been registered yet");
            return;
        }

        if (!isPacketArgsValid<args_t...>(id)) {
            LOG_WARNING("[Client] Args of callback function are not valid", id);
            return;
        }

        if (m_Callbacks.find(id) != m_Callbacks.end()) {
            LOG_WARNING("[Client] Overriding callback with id", id);
        }

        m_Callbacks[id] = [callback](sf::Packet packet) {
            callback(packetReader<args_t>(packet)...);
        };
    }

    ID_t getClientID() const { return m_ClientID; }

private:
    void clientThread();

    sf::TcpSocket m_Socket;

    std::atomic<bool> m_ClientThreadRunning = false;
    std::thread m_ClientThread;

    std::unordered_map<ID_t, ClientReceiveInternalCallback> m_Callbacks;
    std::mutex m_ReceivedPacketsMutex;
    std::vector<sf::Packet> m_ReceivedPackets;

    DisconnectionCallback m_DisconnectionCallback;

    sf::IpAddress m_Ip;
    uint16_t m_Port;

    ID_t m_ClientID = ID_t_MAX;
};
}
