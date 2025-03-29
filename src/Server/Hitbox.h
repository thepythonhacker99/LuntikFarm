#pragma once

#include "SFML/Graphics/Rect.hpp"

// hitbox is aligned with the bottom center of the object
struct Hitbox {
    float sizeX;
    float sizeY;

    Hitbox() = default;

    Hitbox(float sizeX, float sizeY) : sizeX(sizeX), sizeY(sizeY) {}

    [[nodiscard]] sf::FloatRect getRect(sf::Vector2f position) const {
        return {{ position.x - sizeX / 2, position.y - sizeY },
                { sizeX,                  sizeY }};
    }
};
