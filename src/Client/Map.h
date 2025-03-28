#pragma once

#include <cstdint>
#include "Renderer/Renderer.h"
#include "Server/MapInfo.h"
#include "entt/entt.hpp"
#include "Utils/Timers.h"

class Map {
public:
    Map(MapInfo *mapInfo);
    ~Map();

    void render(double dt, Renderer& renderer, entt::registry &registry);

    sf::Texture m_GrassTexture;
    sf::Texture m_CastleTexture;
    sf::Texture m_FarmEmptyTexture;
    sf::Texture m_FarmCollectTexture;

    sf::Texture m_Wall0000;
    sf::Texture m_Wall0001;
    sf::Texture m_Wall0010;
    sf::Texture m_Wall0011;
    sf::Texture m_Wall0100;
    sf::Texture m_Wall0101;
    sf::Texture m_Wall0110;
    sf::Texture m_Wall0111;
    sf::Texture m_Wall1000;
    sf::Texture m_Wall1001;
    sf::Texture m_Wall1010;
    sf::Texture m_Wall1011;
    sf::Texture m_Wall1100;
    sf::Texture m_Wall1101;
    sf::Texture m_Wall1110;
    sf::Texture m_Wall1111;

    sf::Texture m_SoldierTexture;
    int m_AnimationIndex = 0;
    Utils::Timers::NonBlockingTimer<2> m_AnimationTimer;

private:
    sf::Texture* m_WallTextures[16] = {
            &m_Wall0000, &m_Wall0001, &m_Wall0010, &m_Wall0011,
            &m_Wall0100, &m_Wall0101, &m_Wall0110, &m_Wall0111,
            &m_Wall1000, &m_Wall1001, &m_Wall1010, &m_Wall1011,
            &m_Wall1100, &m_Wall1101, &m_Wall1110, &m_Wall1111
    };

    MapInfo *m_MapInfo;
};
