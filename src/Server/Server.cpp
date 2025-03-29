#include "Server.h"
#include "Utils/Timers.h"
#include "Packets.h"
#include "NetworkEntityMap.h"
#include "Farm.h"
#include "Networking/Overloads.h"
#include "Soldier.h"
#include "Hitbox.h"

#include <cmath>

Server::Server(sf::IpAddress ip, uint16_t port) : m_Ip(ip), m_Port(port), m_SocketServer(ip, port) {
    m_IsRunning = false;
}

Server::~Server() {
    if (m_IsRunning) stop();
}

void Server::start() {
    LOG_INFO("Starting server on", m_Ip, ":", m_Port);
    if (m_IsRunning) {
        LOG_WARNING("Server is already running");
        return;
    }

    m_GameState.NEP.init(m_GameState.registry);
    m_GameState.registry.on_construct<Structure>().connect<&Server::onCreateStructure>(this);
    m_GameState.registry.on_destroy<Structure>().connect<&Server::onDeleteStructure>(this);

    m_GameState.registry.on_construct<Farm>().connect<&Server::onUpdateFarm>(this);
    m_GameState.registry.on_update<Farm>().connect<&Server::onUpdateFarm>(this);

    m_GameState.registry.on_construct<Soldier>().connect<&Server::onCreateSoldier>(this);
    m_GameState.registry.on_destroy<Soldier>().connect<&Server::onDeleteSoldier>(this);

    m_SocketServer.setClientConnectedCallback([this](ID_t id) {
        if (m_GameState.gameStage != LOBBY) {
            LOG_WARNING("Client", id, "tried to connect but game stage is not lobby");
            m_SocketServer.kickClient(id);
            return;
        }

        m_GameState.players.emplace(
                id,
                ServerPlayerInfo{ .name = "", .id = id, .ready = false }
        );

        for (const auto& [sendId, info]: m_GameState.players) {
            if (!info.isReady()) continue;
            m_SocketServer.send(id, Networking::createPacket<S2C_PLAYER_PACKET>(sendId, info));
        }

        LOG_INFO("Client connected:", id);
    });

    m_SocketServer.setClientDisconnectedCallback([this](ID_t id) {
        if (m_GameState.gameStage == LOBBY) {
            m_GameState.players.erase(id);
            m_SocketServer.sendAll(Networking::createPacket<S2C_PLAYER_QUIT_PACKET>(id));
        }

        LOG_INFO("Client disconnected:", id);
    });

    m_SocketServer.addReceiveCallback(
            C2S_READY_PACKET,
            std::function<void(ID_t, bool)>([this](ID_t id, bool ready) {
                if (m_GameState.gameStage != LOBBY) {
                    LOG_WARNING("Client", id, " tried to set ready but game stage is not lobby");
                    return;
                }

                if (!m_GameState.players[id].isReady()) {
                    LOG_WARNING("Client", id, " tried to set ready but is not ready");
                    return;
                }

                m_GameState.players[id].ready = ready;
                m_SocketServer.sendAll(Networking::createPacket<S2C_READY_PACKET>(id, ready));

                bool allReady = true;
                for (const auto& [sendId, info]: m_GameState.players) {
                    if (info.isReady() && !info.ready) {
                        allReady = false;
                        break;
                    }
                }

                if (allReady) {
                    m_GameState.gameStage = GAME;
                    LOG_INFO("Game started!");

                    for (const auto& [sendId, info]: m_GameState.players) {
                        if (!info.isReady()) {
                            m_SocketServer.kickClient(info.id);
                        }
                    }

                    m_SocketServer.sendAll(Networking::createPacket<S2C_START_GAME_PACKET>());

                    m_GameState.mapInfo.size = 32;
                    m_GameState.mapInfo.init();

                    {
                        int i = 0;
                        for (auto& [sendId, info]: m_GameState.players) {
                            info.gold = 100000;
                            m_SocketServer.send(sendId, Networking::createPacket<S2C_GOLD_PACKET>(info.gold));

                            auto castle = m_GameState.registry.create();
                            m_GameState.registry.emplace<NetworkID>(castle, m_GameState.networkId++);
                            m_GameState.registry.emplace<Structure>(
                                    castle,
                                    Structure{
                                            .type = StructureType::CASTLE,
                                            .x = 2 + (i % 2) * 10,
                                            .y = 2 + (int) std::floor(i / 2) * 10,
                                            .size = 2,
                                            .owner = sendId
                                    }
                            );

                            auto farm = m_GameState.registry.create();
                            m_GameState.registry.emplace<NetworkID>(farm, m_GameState.networkId++);
                            m_GameState.registry.emplace<Structure>(
                                    farm,
                                    Structure{
                                            .type = StructureType::FARM,
                                            .x = 2 + (i % 2) * 10,
                                            .y = 5 + (int) std::floor(i / 2) * 10,
                                            .size = 1,
                                            .owner = sendId
                                    }
                            );
                            m_GameState.registry.emplace<Farm>(farm);

                            auto farm2 = m_GameState.registry.create();
                            m_GameState.registry.emplace<NetworkID>(farm2, m_GameState.networkId++);
                            m_GameState.registry.emplace<Structure>(
                                    farm2,
                                    Structure{
                                            .type = StructureType::FARM,
                                            .x = 3 + (i % 2) * 10,
                                            .y = 5 + (int) std::floor(i / 2) * 10,
                                            .size = 1,
                                            .owner = sendId
                                    }
                            );
                            m_GameState.registry.emplace<Farm>(farm2);

                            // walls
                            auto wall = m_GameState.registry.create();
                            m_GameState.registry.emplace<NetworkID>(wall, m_GameState.networkId++);
                            m_GameState.registry.emplace<Structure>(
                                    wall,
                                    Structure{
                                            .type = StructureType::WALL,
                                            .x = 2 + (i % 2) * 10,
                                            .y = 6 + (int) std::floor(i / 2) * 10,
                                            .size = 1,
                                            .owner = sendId
                                    }
                            );

                            auto wall2 = m_GameState.registry.create();
                            m_GameState.registry.emplace<NetworkID>(wall2, m_GameState.networkId++);
                            m_GameState.registry.emplace<Structure>(
                                    wall2,
                                    Structure{
                                            .type = StructureType::WALL,
                                            .x = 3 + (i % 2) * 10,
                                            .y = 6 + (int) std::floor(i / 2) * 10,
                                            .size = 1,
                                            .owner = sendId
                                    }
                            );

                            auto wall7 = m_GameState.registry.create();
                            m_GameState.registry.emplace<NetworkID>(wall7, m_GameState.networkId++);
                            m_GameState.registry.emplace<Structure>(
                                    wall7,
                                    Structure{
                                            .type = StructureType::WALL,
                                            .x = 2 + (i % 2) * 10,
                                            .y = 4 + (int) std::floor(i / 2) * 10,
                                            .size = 1,
                                            .owner = sendId
                                    }
                            );

                            auto wall8 = m_GameState.registry.create();
                            m_GameState.registry.emplace<NetworkID>(wall8, m_GameState.networkId++);
                            m_GameState.registry.emplace<Structure>(
                                    wall8,
                                    Structure{
                                            .type = StructureType::WALL,
                                            .x = 3 + (i % 2) * 10,
                                            .y = 4 + (int) std::floor(i / 2) * 10,
                                            .size = 1,
                                            .owner = sendId
                                    }
                            );

                            auto wall3 = m_GameState.registry.create();
                            m_GameState.registry.emplace<NetworkID>(wall3, m_GameState.networkId++);
                            m_GameState.registry.emplace<Structure>(
                                    wall3,
                                    Structure{
                                            .type = StructureType::WALL,
                                            .x = 4 + (i % 2) * 10,
                                            .y = 6 + (int) std::floor(i / 2) * 10,
                                            .size = 1,
                                            .owner = sendId
                                    }
                            );

                            auto wall4 = m_GameState.registry.create();
                            m_GameState.registry.emplace<NetworkID>(wall4, m_GameState.networkId++);
                            m_GameState.registry.emplace<Structure>(
                                    wall4,
                                    Structure{
                                            .type = StructureType::WALL,
                                            .x = 4 + (i % 2) * 10,
                                            .y = 5 + (int) std::floor(i / 2) * 10,
                                            .size = 1,
                                            .owner = sendId
                                    }
                            );

                            auto wall5 = m_GameState.registry.create();
                            m_GameState.registry.emplace<NetworkID>(wall5, m_GameState.networkId++);
                            m_GameState.registry.emplace<Structure>(
                                    wall5,
                                    Structure{
                                            .type = StructureType::WALL,
                                            .x = 4 + (i % 2) * 10,
                                            .y = 4 + (int) std::floor(i / 2) * 10,
                                            .size = 1,
                                            .owner = sendId
                                    }
                            );

                            auto wall6 = m_GameState.registry.create();
                            m_GameState.registry.emplace<NetworkID>(wall6, m_GameState.networkId++);
                            m_GameState.registry.emplace<Structure>(
                                    wall6,
                                    Structure{
                                            .type = StructureType::WALL,
                                            .x = 1 + (i % 2) * 10,
                                            .y = 4 + (int) std::floor(i / 2) * 10,
                                            .size = 1,
                                            .owner = sendId
                                    }
                            );

                            auto wall9 = m_GameState.registry.create();
                            m_GameState.registry.emplace<NetworkID>(wall9, m_GameState.networkId++);
                            m_GameState.registry.emplace<Structure>(
                                    wall9,
                                    Structure{
                                            .type = StructureType::WALL,
                                            .x = 1 + (i % 2) * 10,
                                            .y = 5 + (int) std::floor(i / 2) * 10,
                                            .size = 1,
                                            .owner = sendId
                                    }
                            );

                            auto wall10 = m_GameState.registry.create();
                            m_GameState.registry.emplace<NetworkID>(wall10, m_GameState.networkId++);
                            m_GameState.registry.emplace<Structure>(
                                    wall10,
                                    Structure{
                                            .type = StructureType::WALL,
                                            .x = 1 + (i % 2) * 10,
                                            .y = 6 + (int) std::floor(i / 2) * 10,
                                            .size = 1,
                                            .owner = sendId
                                    }
                            );

                            // add a soldier
                            float soldierSize = 32.f;
                            auto soldier = m_GameState.registry.create();
                            m_GameState.registry.emplace<NetworkID>(soldier, m_GameState.networkId++);
                            m_GameState.registry.emplace<Position>(soldier,
                                                                   32.f * (3 + float(i % 2) * 10.f),
                                                                   32.f * (8 + (float) std::floor(i / 2) * 10.f)
                            );
                            m_GameState.registry.emplace<Soldier>(
                                    soldier,
                                    Soldier(
                                            sendId,
                                            SoldierType::Basic,
                                            soldierSize
                                    )
                            );
                            m_GameState.registry.emplace<Hitbox>(soldier, Hitbox(soldierSize, soldierSize / 2.f));

                            i++;
                        }
                    }
                }
            })
    );

    m_SocketServer.addReceiveCallback(
            C2S_NAME_PACKET,
            std::function<void(ID_t, std::string)>([this](ID_t id, std::string name) {
                if (!m_GameState.players[id].name.empty()) {
                    LOG_WARNING("Client", id, "already has name:", m_GameState.players[id].name);
                    return;
                }

                m_GameState.players[id].name = name;

                if (m_GameState.players[id].isReady())
                    m_SocketServer.sendAll(Networking::createPacket<S2C_PLAYER_PACKET>(id, m_GameState.players[id]));

                LOG_INFO("Client", id, "set name to:", name);
            })
    );

    m_SocketServer.addReceiveCallback(
            C2S_HARVEST_PACKET,
            std::function<void(ID_t, NetworkID)>([this](ID_t sender, NetworkID farmId) {
                if (m_GameState.gameStage != GAME) {
                    LOG_WARNING("Client", sender, " tried to harvest but game stage is not game");
                    return;
                }

                entt::entity entity = m_GameState.NEP.get(farmId.id);
                auto& farm = m_GameState.registry.get<Farm>(entity);
                auto& structure = m_GameState.registry.get<Structure>(entity);

                if (farm.state != FarmState::HARVEST) return;
                if (structure.owner != sender) return;

                m_GameState.registry.patch<Farm>(
                        entity,
                        [](Farm& f) {
                            f.state = FarmState::GROWING;
                        }
                );

                m_GameState.players[sender].gold += 100;
                m_SocketServer.send(
                        sender,
                        Networking::createPacket<S2C_GOLD_PACKET>(m_GameState.players[sender].gold)
                );
            })
    );

    m_SocketServer.addReceiveCallback(
            C2S_PLACE_WALL_PACKET,
            std::function<void(ID_t, int, int)>([this](ID_t sender, int x, int y) {
                if (m_GameState.gameStage != GAME) {
                    LOG_WARNING("Client", sender, "tried to place wall but game stage is not game");
                    return;
                }

                if (x < 0 || x >= m_GameState.mapInfo.size || y < 0 || y >= m_GameState.mapInfo.size) {
                    LOG_WARNING("Client", sender, "tried to place wall out of bounds");
                    return;
                }

                if (m_GameState.mapInfo.structures[y][x] != entt::null) {
                    LOG_WARNING("Client", sender, "tried to place wall on occupied space");
                    return;
                }

                if (m_GameState.players[sender].gold < 10) {
                    return;
                }

                m_GameState.players[sender].gold -= 10;
                m_SocketServer.send(
                        sender,
                        Networking::createPacket<S2C_GOLD_PACKET>(m_GameState.players[sender].gold)
                );

                auto wall = m_GameState.registry.create();
                m_GameState.registry.emplace<NetworkID>(wall, m_GameState.networkId++);
                m_GameState.registry.emplace<Structure>(
                        wall,
                        Structure{
                                .type = StructureType::WALL,
                                .x = x,
                                .y = y,
                                .size = 1,
                                .owner = sender
                        }
                );
            })
    );

    m_SocketServer.addReceiveCallback(
            C2S_PLANT_FARM_PACKET,
            std::function<void(ID_t, int, int)>([this](ID_t sender, int x, int y) {
                if (m_GameState.gameStage != GAME) {
                    LOG_WARNING("Client", sender, "tried to place wall but game stage is not game");
                    return;
                }

                if (x < 0 || x >= m_GameState.mapInfo.size || y < 0 || y >= m_GameState.mapInfo.size) {
                    LOG_WARNING("Client", sender, "tried to place wall out of bounds");
                    return;
                }

                if (m_GameState.mapInfo.structures[y][x] != entt::null) {
                    LOG_WARNING("Client", sender, "tried to place wall on occupied space");
                    return;
                }

                if (m_GameState.players[sender].gold < 100) {
                    return;
                }

                m_GameState.players[sender].gold -= 100;
                m_SocketServer.send(
                        sender,
                        Networking::createPacket<S2C_GOLD_PACKET>(m_GameState.players[sender].gold)
                );

                auto farm = m_GameState.registry.create();
                m_GameState.registry.emplace<NetworkID>(farm, m_GameState.networkId++);
                m_GameState.registry.emplace<Structure>(
                        farm,
                        Structure{
                                .type = StructureType::FARM,
                                .x = x,
                                .y = y,
                                .size = 1,
                                .owner = sender
                        }
                );
                m_GameState.registry.emplace<Farm>(farm);
            })
    );

    m_SocketServer.addReceiveCallback(
            C2S_SPAWN_SOLDIER_PACKET,
            std::function<void(ID_t, float, float)>([this](ID_t sender, float x, float y) {
                if (m_GameState.gameStage != GAME) {
                    LOG_WARNING("Client", sender, "tried to place wall but game stage is not game");
                    return;
                }

                if (x < 0 || x >= m_GameState.mapInfo.size * 32 || y < 0 || y >= m_GameState.mapInfo.size * 32) {
                    LOG_WARNING("Client", sender, "tried to place wall out of bounds");
                    return;
                }

                if (m_GameState.players[sender].gold < 100) {
                    return;
                }

                m_GameState.players[sender].gold -= 100;
                m_SocketServer.send(
                        sender,
                        Networking::createPacket<S2C_GOLD_PACKET>(m_GameState.players[sender].gold)
                );

                auto soldier = m_GameState.registry.create();
                m_GameState.registry.emplace<NetworkID>(soldier, m_GameState.networkId++);
                m_GameState.registry.emplace<Position>(soldier, x, y);
                m_GameState.registry.emplace<Soldier>(
                        soldier,
                        Soldier(
                                sender,
                                SoldierType::Basic,
                                32.f
                        ));
            })
    );

    m_SocketServer.start();
    if (!m_SocketServer.isListenThreadRunning()) {
        LOG_WARNING("Failed to start socket server thread");
        return;
    }

    m_IsRunning = true;
    LOG_INFO("Server started");
}

void Server::stop() {
    LOG_INFO("Stopping server");
    if (!m_IsRunning) {
        LOG_WARNING("Server isn't running");
        return;
    }

    m_SocketServer.stop();
    m_IsRunning = false;
    LOG_INFO("Server stopped");
}

void Server::tick(double deltaTime) {
    if (!m_IsRunning) {
        LOG_WARNING("Server isn't running");
        return;
    }

    m_SocketServer.handleCallbacks();

    bool updatePositions = m_PositionUpdateTimer.timeReached(deltaTime);

    {
        auto view = m_GameState.registry.view<Farm>();
        for (auto entity: view) {
            auto& farm = view.get<Farm>(entity);
            if (farm.state == FarmState::HARVEST) {
                continue;
            }

            farm.time++;
            if (farm.time >= farm.growTime) {
                m_GameState.registry.patch<Farm>(
                        entity,
                        [](Farm& f) {
                            f.time = 0;
                            f.state = FarmState::HARVEST;
                        }
                );
            }
        }
    }

    {
        auto view = m_GameState.registry.view<Soldier, Position, NetworkID>();
        view.each([&](auto entity, auto& soldier, auto& position, auto& networkID) {
            float velocity = deltaTime * 32.f * 3;

            sf::Vector2f direction;

            // --- SOLDIER AI ---

            // --- SOLDIER AI ---

            if (direction.x != 0 && direction.y != 0) {
                direction = direction.normalized();

                position.x += direction.x * velocity;
                position.y += direction.y * velocity;

                if (updatePositions) {
                    m_SocketServer.sendAll(Networking::createPacket<S2C_SOLDIER_POSITION_PACKET>(networkID, position));
                }
            }
        });
    }
}

void Server::run() {
    LOG_INFO("Running server");
    if (!m_IsRunning) {
        LOG_WARNING("Server isn't running");
        return;
    }

    sf::Clock clock;
    float deltaTime;
    int fps = 0;
    float timeForFps = 0.f;

    Utils::Timers::BlockingTimer<20> tickTimer;

    while (m_IsRunning) {
        deltaTime = clock.restart().asSeconds();

        timeForFps += deltaTime;
        fps++;
        if (timeForFps >= 1.f) {
//            LOG_INFO("FPS:", fps);
            timeForFps -= 1.f;
            fps = 0;
        }

        tick(deltaTime);
        tickTimer.sleep();
    }
}