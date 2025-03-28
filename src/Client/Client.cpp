#include "Client.h"

#include <utility>
#include "Utils/Timers.h"
#include "Packets.h"
#include "ClientGameState.h"
#include "NetworkEntityMap.h"
#include "Renderer/YSort.h"
#include "Server/Position.h"
#include "InterpolatedPosition.h"

Client::Client(sf::IpAddress ip, uint16_t port, std::string name) : m_Ip(ip), m_Port(port),
                                                                    m_SocketClient(ip, port),
                                                                    m_Renderer("Luntik Farm"), m_Name(std::move(name)),
                                                                    m_GameState(), m_Map(&m_GameState.mapInfo) {
    m_IsRunning = false;
}

Client::~Client() {
    if (m_IsRunning) stop();
}

void Client::start() {
    LOG_INFO("Starting client on", m_Ip, ":", m_Port);
    if (m_IsRunning) {
        LOG_WARNING("Client is already running");
        return;
    }

    if (!m_ShopTexture.loadFromFile("assets/shop.png")) {
        LOG_WARNING("Failed to load shop texture");
    }

    m_GameState.NEP.init(m_GameState.registry);

    m_GameState.registry.on_construct<Structure>().connect<&Client::onCreateStructure>(this);
    m_GameState.registry.on_destroy<Structure>().connect<&Client::onDeleteStructure>(this);

    m_SocketClient.setDisconnectionCallback([this]() {
        LOG_WARNING("Disconnected from server");
        stop();
    });

    m_SocketClient.addReceiveCallback(
            S2C_PLAYER_PACKET,
            std::function<void(ID_t, ServerPlayerInfo)>([this](ID_t id, const ServerPlayerInfo& info) {
                m_GameState.players.insert_or_assign(id, info);
                LOG_INFO("Added to lobby:", m_GameState.players[id].name);
            })
    );

    m_SocketClient.addReceiveCallback(
            S2C_PLAYER_QUIT_PACKET,
            std::function<void(ID_t)>([this](ID_t id) {
                m_GameState.players.erase(id);
                LOG_INFO("Removed from lobby:", id);
            })
    );

    m_SocketClient.addReceiveCallback(
            S2C_LOBBY_PACKET,
            std::function<void(std::unordered_map<ID_t, ServerPlayerInfo>)>(
                    [this](std::unordered_map<ID_t, ServerPlayerInfo> lobby) {
                        m_GameState.players = std::move(lobby);
                        LOG_INFO("Set lobby");
                    })
    );

    m_SocketClient.addReceiveCallback(
            S2C_READY_PACKET,
            std::function<void(ID_t, bool)>([this](ID_t id, bool ready) {
                m_GameState.players[id].ready = ready;
                LOG_INFO("Set ready:", ready);
            })
    );

    m_SocketClient.addReceiveCallback(
            S2C_START_GAME_PACKET,
            std::function<void()>([this]() {
                m_GameState.gameStage = GameStage::GAME;
                m_GameState.mapInfo.size = 32;
                m_GameState.mapInfo.init();
                LOG_INFO("Game started");
            })
    );

    m_SocketClient.addReceiveCallback(
            S2C_GOLD_PACKET,
            std::function<void(int)>([this](int gold) {
                m_GameState.players[m_SocketClient.getClientID()].gold = gold;
            })
    );

    m_SocketClient.addReceiveCallback(
            S2C_STRUCTURE_PACKET,
            std::function<void(NetworkID, Structure)>([this](NetworkID id, Structure structure) {
                auto structureEntity = m_GameState.registry.create();
                m_GameState.registry.emplace<NetworkID>(structureEntity, id.id);
                m_GameState.registry.emplace<Structure>(structureEntity, structure);
                m_GameState.registry
                        .emplace<YSort>(structureEntity, 32 * (structure.y + structure.size));
            })
    );

    m_SocketClient.addReceiveCallback(
            S2C_STRUCTURE_DELETE_PACKET,
            std::function<void(NetworkID)>([this](NetworkID id) {
                m_GameState.registry.destroy(m_GameState.NEP.get(id.id));
            })
    );

    m_SocketClient.addReceiveCallback(
            S2C_FARM_PACKET,
            std::function<void(NetworkID, Farm)>([this](NetworkID id, Farm farm) {
                m_GameState.registry.emplace_or_replace<Farm>(m_GameState.NEP.get(id.id), farm);
            })
    );

    m_SocketClient.addReceiveCallback(
            S2C_SOLDIER_CREATE_PACKET,
            std::function<void(NetworkID, Soldier, Position)>([this](NetworkID id, Soldier soldier, Position pos) {
                auto soldierEntity = m_GameState.registry.create();
                m_GameState.registry.emplace<NetworkID>(soldierEntity, id.id);
                m_GameState.registry.emplace<Soldier>(soldierEntity, soldier);
                m_GameState.registry.emplace<InterpolatedPosition>(soldierEntity, pos.x, pos.y, 0.15f);
                m_GameState.registry.emplace<YSort>(soldierEntity, pos.y);
            })
    );

    m_SocketClient.addReceiveCallback(
            S2C_SOLDIER_DELETE_PACKET,
            std::function<void(NetworkID)>([this](NetworkID id) {
                m_GameState.registry.destroy(m_GameState.NEP.get(id.id));
            })
    );

    m_SocketClient.addReceiveCallback(
            S2C_SOLDIER_POSITION_PACKET,
            std::function<void(NetworkID, Position)>([this](NetworkID id, Position pos) {
                m_GameState.registry.get<InterpolatedPosition>(m_GameState.NEP.get(id.id)).set(pos.x, pos.y);
            })
    );

    m_SocketClient.start();
    if (!m_SocketClient.isRunning()) {
        LOG_WARNING("Failed to start socket client!");
        return;
    }

    m_IsRunning = true;
    LOG_INFO("Client started");
}

void Client::stop() {
    LOG_INFO("Stopping client");
    if (!m_IsRunning) {
        LOG_WARNING("Client isn't running");
        return;
    }

    m_SocketClient.stop();
    m_IsRunning = false;
    LOG_INFO("Client stopped");
}

void Client::tick(double deltaTime) {
    if (!m_IsRunning) {
        LOG_WARNING("Client isn't running");
        return;
    }

    m_InputManager.update(m_Renderer.window());

    if (m_FocusTarget != FocusTarget::TARGET_NONE && m_InputManager.isPressed(sf::Keyboard::Key::Escape)) {
        m_FocusTarget = FocusTarget::TARGET_NONE;
        return;
    }

    m_SineTime += deltaTime;
    if (m_SineTime > 2 * M_PI) m_SineTime -= 2 * M_PI;

    m_SocketClient.handleCallbacks();

    m_Renderer.update();
    m_Renderer.window().clear(sf::Color::Black);

    switch (m_GameState.gameStage) {
        case GameStage::LOBBY: {
            m_Renderer.setViewUI();

            auto lobbyText = sf::Text(m_Renderer.font(), "LOBBY", 50);
            lobbyText.setPosition(m_Renderer.uiTopLeft() + sf::Vector2f{ 10.f, 10.f });
            lobbyText.setStyle(sf::Text::Style::Bold);
            m_Renderer.window().draw(lobbyText);

            {
                int i = 0;
                for (auto& [id, player]: m_GameState.players) {
                    auto text = sf::Text(m_Renderer.font());
                    if (id == m_SocketClient.getClientID()) text.setString(player.name + " (YOU)");
                    else text.setString(player.name);
                    text.setFillColor(player.ready ? sf::Color::Green : sf::Color::White);
                    text.setPosition(m_Renderer.uiTopLeft() + sf::Vector2f{ 40, 100.f + i * 50 });
                    m_Renderer.window().draw(text);
                    ++i;
                }
            }

            if (!m_GameState.players[m_SocketClient.getClientID()].ready) {
                auto text = sf::Text(m_Renderer.font(), "Press SPACE to be ready");
                text.setFillColor(sf::Color::Red);
                text.setPosition(m_Renderer.uiBottomCenter() - sf::Vector2f{ 0, 20.f });
                text.setOrigin(sf::Vector2f{ text.getGlobalBounds().size.x / 2.f, text.getGlobalBounds().size.y });
                m_Renderer.window().draw(text);
            } else {
                auto text = sf::Text(m_Renderer.font(), "Ready");
                text.setFillColor(sf::Color::Green);
                text.setPosition(m_Renderer.uiBottomCenter() - sf::Vector2f{ 0, 20.f });
                text.setOrigin(sf::Vector2f{ text.getGlobalBounds().size.x / 2.f, text.getGlobalBounds().size.y });
                m_Renderer.window().draw(text);
            }

            break;
        }
        case GAME: {
            if (m_FocusTarget == TARGET_NONE || m_FocusTarget == TARGET_BUILDING || m_FocusTarget == TARGET_SPAWN) {
                sf::Vector2f cameraDelta;
                if (m_InputManager.isDown(sf::Keyboard::Key::A)) cameraDelta.x -= 1;
                if (m_InputManager.isDown(sf::Keyboard::Key::D)) cameraDelta.x += 1;
                if (m_InputManager.isDown(sf::Keyboard::Key::W)) cameraDelta.y -= 1;
                if (m_InputManager.isDown(sf::Keyboard::Key::S)) cameraDelta.y += 1;
                if (cameraDelta != sf::Vector2f{ 0, 0 })
                    m_Renderer.viewMain().move(cameraDelta.normalized() * 1000.f * (float) deltaTime);
            }

            m_Map.render(deltaTime, m_Renderer, m_GameState.registry);

            m_Renderer.setViewUI();
            auto goldText = sf::Text(m_Renderer.font(), "", 50);
            goldText.setString("Gold: " + std::to_string(m_GameState.players[m_SocketClient.getClientID()].gold));
            goldText.setPosition(m_Renderer.uiTopLeft() + sf::Vector2f{ 10.f, 10.f });
            goldText.setStyle(sf::Text::Style::Bold);
            goldText.setFillColor(sf::Color::Yellow);
            m_Renderer.window().draw(goldText);

            // get mouse position in world view
            sf::Vector2i mousePos = sf::Mouse::getPosition(m_Renderer.window());
            sf::Vector2f worldPos = m_Renderer.window().mapPixelToCoords(mousePos, m_Renderer.viewMain());
            sf::Vector2f uiPos = m_Renderer.window().mapPixelToCoords(mousePos, m_Renderer.viewUI());

            int tileX = static_cast<int>(std::floor(worldPos.x / 32.f));
            int tileY = static_cast<int>(std::floor(worldPos.y / 32.f));

            {
                m_GameState.registry.view<InterpolatedPosition>().each(
                        [deltaTime](auto& position) { position.update(deltaTime); }
                );
            }

            switch (m_FocusTarget) {
                case TARGET_NONE: {
                    if (tileX >= 0 && tileX < m_GameState.mapInfo.size && tileY >= 0 &&
                        tileY < m_GameState.mapInfo.size) {
                        entt::entity hoverEntity = m_GameState.mapInfo.structures[tileY][tileX];
                        if (hoverEntity != entt::null) {
                            Structure& structure = m_GameState.registry.get<Structure>(hoverEntity);
                            switch (structure.type) {
                                case StructureType::FARM: {
                                    if (m_InputManager.isReleased(sf::Mouse::Button::Left)) {
                                        auto& farmComponent = m_GameState.registry.get<Farm>(hoverEntity);
                                        auto& networkComponent = m_GameState.registry.get<NetworkID>(hoverEntity);
                                        if (farmComponent.state == HARVEST) {
                                            m_SocketClient
                                                    .send(Networking::createPacket<C2S_HARVEST_PACKET>(
                                                            networkComponent));
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }

                    if (m_InputManager.isPressed(sf::Keyboard::Key::B)) {
                        m_FocusTarget = FocusTarget::TARGET_SHOP;
                        m_SelectedShopItem = nullptr;
                    }
                    break;
                }
                case TARGET_SHOP: {
                    // draw shop
                    sf::Sprite shopSprite(m_ShopTexture);
                    shopSprite.setPosition({ 0, 0 });
                    shopSprite.setOrigin({ 600, 300 });
                    m_Renderer.setViewUI();
                    m_Renderer.window().draw(shopSprite);

                    for (int i = 0; i < m_ShopSizeY; i++) {
                        for (int j = 0; j < m_ShopSizeX; j++) {
                            int index = i * m_ShopSizeX + j;
                            if (index >= s_ShopItems.size()) break;

                            const ShopItem& item = s_ShopItems[index];

                            sf::Vector2f pos = sf::Vector2f{
                                    static_cast<float>(20.f + j * 236),
                                    static_cast<float>(20.f + i * 290)
                            } - shopSprite.getOrigin();
                            sf::Vector2f size{ 216, 270 };

                            sf::FloatRect rect{ pos, size };
                            bool hovered = rect.contains(uiPos);

                            sf::RectangleShape itemSprite;
                            itemSprite.setPosition(pos);
                            itemSprite.setSize(size);
                            itemSprite.setFillColor(sf::Color(0, 0, 0, hovered ? 50 : 20));
                            itemSprite.setOutlineColor(sf::Color::Black);
                            itemSprite.setOutlineThickness(3.f);
                            if (hovered) itemSprite.setOutlineThickness(5.f);

                            m_Renderer.window().draw(itemSprite);

                            // total 270
                            // 10 margin
                            // 30 text
                            // 10 margin
                            // 170 image
                            // 10 margin
                            // 30 price
                            // 10 margin

                            sf::Text itemText(m_Renderer.font(), item.name, 30);
                            itemText.setPosition(pos + sf::Vector2f{ 108.f, 10.f });
                            itemText.setOrigin({ itemText.getLocalBounds().size.x / 2, 0 });
                            itemText.setFillColor(sf::Color::White);
                            m_Renderer.window().draw(itemText);

                            sf::Sprite itemImage(m_Map.m_Wall0000);
                            switch (item.id) {
                                case ShopId::FARM: {
                                    itemImage = sf::Sprite(m_Map.m_FarmCollectTexture);
                                    break;
                                }
                                case ShopId::WALL: {
                                    itemImage = sf::Sprite(m_Map.m_Wall0000);
                                    break;
                                }
                                case ShopId::SOLDIER: {
                                    itemImage = sf::Sprite(m_Map.m_SoldierTexture);
                                    itemImage.setTextureRect({{ 0,   0 },
                                                              { 320, 320 }});
                                    itemImage.setScale({ 0.1f, 0.1f });
                                    break;
                                }
                            }
                            itemImage.setScale(itemImage.getScale().componentWiseMul(sf::Vector2f{ 85.f, 85.f } / itemImage.getGlobalBounds().size.x));
                            itemImage.setPosition(pos + sf::Vector2f{ 108.f, 220.f });
                            itemImage.setOrigin(
                                    { itemImage.getLocalBounds().size.x / 2, itemImage.getLocalBounds().size.y });
                            m_Renderer.window().draw(itemImage);

                            sf::Text priceText(m_Renderer.font(), std::to_string(item.price), 30);
                            priceText.setPosition(pos + sf::Vector2f{ 108.f, 230.f });
                            priceText.setOrigin({ priceText.getLocalBounds().size.x / 2, 0 });
                            priceText.setFillColor(sf::Color::Yellow);
                            m_Renderer.window().draw(priceText);

                            if (m_InputManager.isPressed(sf::Mouse::Button::Left) && hovered) {
                                switch (item.id) {
                                    case ShopId::FARM:
                                    case ShopId::WALL:
                                        m_FocusTarget = FocusTarget::TARGET_BUILDING;
                                        break;

                                    case ShopId::SOLDIER:
                                        m_FocusTarget = FocusTarget::TARGET_SPAWN;
                                        break;
                                }

                                m_SelectedShopItem = &s_ShopItems[index];
                            }
                        }
                    }

                    if (m_InputManager.isPressed(sf::Keyboard::Key::B)) {
                        m_FocusTarget = FocusTarget::TARGET_NONE;
                    }
                    break;
                }
                case TARGET_BUILDING: {
                    if (tileX >= 0 && tileX < m_GameState.mapInfo.size && tileY >= 0 &&
                        tileY < m_GameState.mapInfo.size) {
                        entt::entity hoverEntity = m_GameState.mapInfo.structures[tileY][tileX];
                        if (hoverEntity == entt::null) {
                            switch (m_SelectedShopItem->id) {
                                case ShopId::WALL: {
                                    // draw a wall
                                    sf::Sprite sprite(m_Map.m_Wall0000);
                                    sprite.setPosition(
                                            { static_cast<float>(tileX) * 32, static_cast<float>(tileY) * 32 });
                                    sprite.setOrigin({ 0, 32 });
                                    sprite.setColor(
                                            sf::Color{ 255, 255, 255,
                                                       static_cast<uint8_t>(20 * std::sin(m_SineTime * 8) + 200) });

                                    m_Renderer.setViewMain();
                                    m_Renderer.window().draw(sprite);

                                    if (m_InputManager.isPressed(sf::Mouse::Button::Left)) {
                                        m_SocketClient
                                                .send(Networking::createPacket<C2S_PLACE_WALL_PACKET>(tileX, tileY));
                                    }
                                    break;
                                }
                                case ShopId::FARM: {
                                    // draw a wall
                                    sf::Sprite sprite(m_Map.m_FarmCollectTexture);
                                    sprite.setPosition(
                                            { static_cast<float>(tileX) * 32, static_cast<float>(tileY + 1) * 32 });
                                    sprite.setOrigin({ 0, sprite.getLocalBounds().size.y });
                                    sprite.setColor(
                                            sf::Color{ 255, 255, 255,
                                                       static_cast<uint8_t>(20 * std::sin(m_SineTime * 8) + 200) });

                                    m_Renderer.setViewMain();
                                    m_Renderer.window().draw(sprite);

                                    if (m_InputManager.isPressed(sf::Mouse::Button::Left)) {
                                        m_SocketClient
                                                .send(Networking::createPacket<C2S_PLANT_FARM_PACKET>(tileX, tileY));
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    break;
                }
                case TARGET_SPAWN: {
                    if (tileX >= 0 && tileX < m_GameState.mapInfo.size && tileY >= 0 &&
                        tileY < m_GameState.mapInfo.size) {

                        sf::Sprite sprite(m_Map.m_SoldierTexture);
                        sprite.setTextureRect({{ 0,   0 },
                                               { 320, 320 }});
                        sprite.setScale({ 0.1f, 0.1f });
                        sprite.setPosition(worldPos);
                        sprite.setOrigin({ 0, 320 });
                        sprite.setColor(
                                sf::Color{ 255, 255, 255,
                                           static_cast<uint8_t>(20 * std::sin(m_SineTime * 8) + 200) });

                        m_Renderer.setViewMain();
                        m_Renderer.window().draw(sprite);

                        if (m_InputManager.isPressed(sf::Mouse::Button::Left)) {
                            m_SocketClient.send(Networking::createPacket<C2S_SPAWN_SOLDIER_PACKET>(worldPos.x, worldPos.y));
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
    m_Renderer.window().display();

    while (const std::optional event = m_Renderer.window().pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            m_Renderer.window().close();
            stop();
        } else if (event->is<sf::Event::KeyPressed>()) {
            if (event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Space &&
                m_GameState.gameStage == LOBBY) {
                m_SocketClient.send(Networking::createPacket<C2S_READY_PACKET>(
                        !m_GameState.players[m_SocketClient.getClientID()].ready));
            }
        }
    }
}

void Client::run() {
    LOG_INFO("Running client");
    if (!m_IsRunning) {
        LOG_WARNING("Client isn't running");
        return;
    }

    sf::Clock clock;
    float deltaTime;
    int fps = 0;
    float timeForFps = 0.f;

    m_Renderer.init();
    m_Renderer.window().setVerticalSyncEnabled(true);

    m_SocketClient.send(Networking::createPacket<C2S_NAME_PACKET>(m_Name));
    while (m_IsRunning) {
        deltaTime = clock.restart().asSeconds();
        timeForFps += deltaTime;
        ++fps;

        if (timeForFps >= 1.f) {
            LOG_INFO("FPS:", fps);
            timeForFps -= 1.f;
            fps = 0;
        }

        tick(deltaTime);
    }
}
