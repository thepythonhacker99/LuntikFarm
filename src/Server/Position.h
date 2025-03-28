#pragma once

#include "SFML/Network/Packet.hpp"

struct Position {
    float x;
    float y;
};

inline sf::Packet& operator<<(sf::Packet& packet, const Position& position) {
    return packet << position.x << position.y;
}

inline sf::Packet& operator>>(sf::Packet& packet, Position& position) {
    return packet >> position.x >> position.y;
}
