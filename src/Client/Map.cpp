#include "Map.h"
#include "logy.h"
#include "Server/Farm.h"
#include "Client/Renderer/YSort.h"
#include "Server/Soldier.h"
#include "Server/Position.h"
#include "InterpolatedPosition.h"
#include "Server/Hitbox.h"
#include "opts.h"

Map::Map(MapInfo *mapInfo) : m_MapInfo(mapInfo) {
    if (!m_GrassTexture.loadFromFile("assets/grass.png")) {
        LOG_WARNING("Failed to load grass texture");
    }

    if (!m_CastleTexture.loadFromFile("assets/castle.png")) {
        LOG_WARNING("Failed to load castle texture");
    }

    if (!m_FarmEmptyTexture.loadFromFile("assets/farm_empty.png")) {
        LOG_WARNING("Failed to load farm texture");
    }

    if (!m_FarmCollectTexture.loadFromFile("assets/farm_collect.png")) {
        LOG_WARNING("Failed to load farm texture");
    }

    if (!m_Wall0000.loadFromFile("assets/fence_0000.png") ||
        !m_Wall0001.loadFromFile("assets/fence_0001.png") ||
        !m_Wall0010.loadFromFile("assets/fence_0010.png") ||
        !m_Wall0011.loadFromFile("assets/fence_0011.png") ||
        !m_Wall0100.loadFromFile("assets/fence_0100.png") ||
        !m_Wall0101.loadFromFile("assets/fence_0101.png") ||
        !m_Wall0110.loadFromFile("assets/fence_0110.png") ||
        !m_Wall0111.loadFromFile("assets/fence_0111.png") ||
        !m_Wall1000.loadFromFile("assets/fence_1000.png") ||
        !m_Wall1001.loadFromFile("assets/fence_1001.png") ||
        !m_Wall1010.loadFromFile("assets/fence_1010.png") ||
        !m_Wall1011.loadFromFile("assets/fence_1011.png") ||
        !m_Wall1100.loadFromFile("assets/fence_1100.png") ||
        !m_Wall1101.loadFromFile("assets/fence_1101.png") ||
        !m_Wall1110.loadFromFile("assets/fence_1110.png") ||
        !m_Wall1111.loadFromFile("assets/fence_1111.png")) {
        LOG_WARNING("Failed to load fence texture");
    }

    if (!m_SoldierTexture.loadFromFile("assets/soldier.png")) {
        LOG_WARNING("Failed to load soldier texture");
    }
}

Map::~Map() {

}

void Map::render(double dt, Renderer& renderer, entt::registry& registry) {
    registry.sort<YSort>([](const YSort& a, const YSort& b) {
        return a.y < b.y;
    });

    renderer.setViewMain();

    for (int y = 0; y < m_MapInfo->size; y++) {
        for (int x = 0; x < m_MapInfo->size; x++) {
            sf::Sprite sprite(m_GrassTexture);
            sprite.setPosition({ static_cast<float>(x) * 32, static_cast<float>(y) * 32 });
            renderer.window().draw(sprite);
        }
    }

    if (m_AnimationTimer.timeReached(dt)) {
        ++m_AnimationIndex %= 4;
    }

    auto drawView = registry.view<YSort>();
    for (auto entity: drawView) {
        auto *structure = registry.try_get<Structure>(entity);
        if (structure) {
            switch (structure->type) {
                case CASTLE: {
                    sf::Sprite sprite(m_CastleTexture);
                    sprite.setPosition(
                            { static_cast<float>(structure->x) * 32, static_cast<float>(structure->y) * 32 });
                    sprite.setOrigin({ 0, sprite.getGlobalBounds().size.y - structure->size * 32 });
                    renderer.window().draw(sprite);
                    break;
                }
                case FARM: {
                    if (!registry.all_of<Farm>(entity)) continue;
                    sf::Sprite sprite(
                            registry.get<Farm>(entity).state == HARVEST ? m_FarmCollectTexture : m_FarmEmptyTexture);
                    sprite.setPosition(
                            { static_cast<float>(structure->x) * 32, static_cast<float>(structure->y) * 32 });
                    renderer.window().draw(sprite);
                    break;
                }
                case WALL: {
                    uint8_t neighbourMask = 0;
                    if (structure->y > 0 && m_MapInfo->structures[structure->y - 1][structure->x] != entt::null) {
                        auto& wall = registry.get<Structure>(m_MapInfo->structures[structure->y - 1][structure->x]);
                        if (wall.type == WALL && structure->owner == wall.owner) {
                            neighbourMask |= 0b1000;
                        }
                    }
                    if (structure->x < m_MapInfo->size - 1 &&
                        m_MapInfo->structures[structure->y][structure->x + 1] != entt::null) {
                        auto& wall = registry.get<Structure>(m_MapInfo->structures[structure->y][structure->x + 1]);
                        if (wall.type == WALL && structure->owner == wall.owner) {
                            neighbourMask |= 0b0100;
                        }
                    }
                    if (structure->y < m_MapInfo->size - 1 &&
                        m_MapInfo->structures[structure->y + 1][structure->x] != entt::null) {
                        auto& wall = registry.get<Structure>(m_MapInfo->structures[structure->y + 1][structure->x]);
                        if (wall.type == WALL && structure->owner == wall.owner) {
                            neighbourMask |= 0b0010;
                        }
                    }
                    if (structure->x > 0 && m_MapInfo->structures[structure->y][structure->x - 1] != entt::null) {
                        auto& wall = registry.get<Structure>(m_MapInfo->structures[structure->y][structure->x - 1]);
                        if (wall.type == WALL && structure->owner == wall.owner) {
                            neighbourMask |= 0b0001;
                        }
                    }

                    sf::Sprite sprite(*m_WallTextures[neighbourMask]);
                    sprite.setPosition(
                            { static_cast<float>(structure->x) * 32, static_cast<float>(structure->y) * 32 });
                    sprite.setOrigin({ 0, 32 });

                    renderer.window().draw(sprite);
                    break;
                }
            }
        }

        auto *soldier = registry.try_get<Soldier>(entity);
        if (soldier) {
            auto position = registry.get<InterpolatedPosition>(entity);
            auto hitbox = registry.get<Hitbox>(entity);

            sf::Sprite sprite(m_SoldierTexture);
            sprite.scale({ 0.1f, 0.1f });
            sprite.setPosition({ position.x, position.y });
            sprite.setOrigin({ 0, static_cast<float>(m_SoldierTexture.getSize().y) });
            sprite.setTextureRect({{ m_AnimationIndex * 320, 0 },
                                   { 320,                    320 }});
            renderer.window().draw(sprite);

#if LTK_DEBUG
            sf::FloatRect hitboxRect = hitbox.getRect({ position.x + soldier->size / 2, position.y });
            sf::RectangleShape hitboxShape;
            hitboxShape.setPosition({ hitboxRect.position.x, hitboxRect.position.y });
            hitboxShape.setSize({ hitboxRect.size.x, hitboxRect.size.y });
            hitboxShape.setFillColor({ 255, 0, 0, 100 });
            renderer.window().draw(hitboxShape);
#endif
        }
    }
}
