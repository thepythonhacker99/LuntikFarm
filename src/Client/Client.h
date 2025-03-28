#pragma once

#include <cstdint>
#include "Networking/SocketClient.h"
#include "SFML/Network/IpAddress.hpp"
#include "ClientGameState.h"
#include "Client/Renderer/Renderer.h"
#include "InputManager.h"

enum class ShopId {
    FARM,
    WALL,
    SOLDIER,
};

struct ShopItem {
    ShopId id;
    std::string name;
    uint16_t price;
};

enum FocusTarget {
    TARGET_NONE,
    TARGET_SHOP,
    TARGET_BUILDING,
    TARGET_SPAWN,
};

class Client {
public:
    Client(sf::IpAddress ip, uint16_t port, std::string name);
    ~Client();

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
    }

    void onDeleteStructure(entt::registry& registry, entt::entity entity) {
        Structure& structureComponent = registry.get<Structure>(entity);

        for (int i = 0; i < structureComponent.size; i++)
            for (int j = 0; j < structureComponent.size; j++)
                m_GameState.mapInfo.structures[structureComponent.y + i][structureComponent.x + j] = entt::null;
    }

    sf::IpAddress m_Ip;
    uint16_t m_Port;
    Networking::SocketClient m_SocketClient;
    std::atomic<bool> m_IsRunning;

    ClientGameState m_GameState;
    InputManager m_InputManager;
    Renderer m_Renderer;
    Map m_Map;

    std::string m_Name;
    float m_SineTime = 0.0f;

    FocusTarget m_FocusTarget = TARGET_NONE;

    sf::Texture m_ShopTexture;
    const ShopItem *m_SelectedShopItem = nullptr;

    static const std::vector<ShopItem> s_ShopItems;
    const int m_ShopSizeX = 5;
    const int m_ShopSizeY = 2;
};

inline const std::vector<ShopItem> Client::s_ShopItems = {
        ShopItem{
                .id = ShopId::FARM,
                .name = "Farm",
                .price = 100
        },
        ShopItem{
                .id = ShopId::WALL,
                .name = "Wall",
                .price = 10
        },
        ShopItem{
                .id = ShopId::SOLDIER,
                .name = "Soldier",
                .price = 100
        }
};
