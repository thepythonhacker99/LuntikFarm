#pragma once

#include <atomic>
#include "SFML/Network/IpAddress.hpp"
#include "Networking/SocketServer.h"
#include "Packets.h"
#include "ServerGameState.h"
#include "NetworkEntityMap.h"
#include "Soldier.h"
#include "Utils/Timers.h"

class Server {
public:
    Server(sf::IpAddress ip, uint16_t port);
    ~Server();

    bool isRunning() const { return m_IsRunning; };

    void start();
    void stop();

    void tick(double deltaTime);
    void run();

private:
    void onCreateStructure(entt::registry& registry, entt::entity entity) {
        Structure& structureComponent = registry.get<Structure>(entity);

        for (int i = 0; i < structureComponent.size; i++)
            for (int j = 0; j < structureComponent.size; j++)
                m_GameState.mapInfo.structures[structureComponent.y + i][structureComponent.x + j] = entity;

        NetworkID *networkIdComponent = registry.try_get<NetworkID>(entity);
        if (!networkIdComponent) {
            LOG_WARNING("Structure has no NetworkID");
            return;
        }

        m_SocketServer.sendAll(Networking::createPacket<S2C_STRUCTURE_PACKET>(*networkIdComponent, structureComponent));
    }

    void onDeleteStructure(entt::registry& registry, entt::entity entity) {
        Structure& structureComponent = registry.get<Structure>(entity);

        for (int i = 0; i < structureComponent.size; i++)
            for (int j = 0; j < structureComponent.size; j++)
                m_GameState.mapInfo.structures[structureComponent.y + i][structureComponent.x + j] = entt::null;

        NetworkID *networkIdComponent = registry.try_get<NetworkID>(entity);
        if (!networkIdComponent) {
            LOG_WARNING("Structure has no NetworkID");
            return;
        }

        m_SocketServer.sendAll(Networking::createPacket<S2C_STRUCTURE_DELETE_PACKET>(*networkIdComponent));
    }

    void onUpdateFarm(entt::registry& registry, entt::entity entity) {
        Farm& farmComponent = registry.get<Farm>(entity);

        NetworkID *networkIdComponent = registry.try_get<NetworkID>(entity);
        if (!networkIdComponent) {
            LOG_WARNING("Farm has no NetworkID");
            return;
        }

        m_SocketServer.sendAll(Networking::createPacket<S2C_FARM_PACKET>(*networkIdComponent, farmComponent));
    }

    void onCreateSoldier(entt::registry& registry, entt::entity entity) {
        Soldier& soldierComponent = registry.get<Soldier>(entity);

        NetworkID *networkIdComponent = registry.try_get<NetworkID>(entity);
        if (!networkIdComponent) {
            LOG_WARNING("Soldier has no NetworkID");
            return;
        }

        Position *positionComponent = registry.try_get<Position>(entity);
        if (!positionComponent) {
            LOG_WARNING("Soldier has no Position");
            return;
        }

        m_SocketServer.sendAll(Networking::createPacket<S2C_SOLDIER_CREATE_PACKET>(*networkIdComponent, soldierComponent, *positionComponent));
    }

    void onDeleteSoldier(entt::registry& registry, entt::entity entity) {
        NetworkID *networkIdComponent = registry.try_get<NetworkID>(entity);
        if (!networkIdComponent) {
            LOG_WARNING("Soldier has no NetworkID");
            return;
        }

        m_SocketServer.sendAll(Networking::createPacket<S2C_SOLDIER_DELETE_PACKET>(*networkIdComponent));
    }

    Networking::SocketServer m_SocketServer;

    sf::IpAddress m_Ip;
    uint16_t m_Port;

    std::atomic<bool> m_IsRunning;

    ServerGameState m_GameState;

    Utils::Timers::NonBlockingTimer<10> m_PositionUpdateTimer;
};
