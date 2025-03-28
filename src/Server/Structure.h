#pragma once

#include "SFML/Network/Packet.hpp"
#include "Utils/Utils.h"

enum StructureType {
    CASTLE,
    FARM,
    WALL
};

struct Structure {
    StructureType type;

    int x;
    int y;

    int size;

    ID_t owner;
};

inline sf::Packet& operator<<(sf::Packet& packet, const StructureType& info) {
    return packet << (int)info;
}

inline sf::Packet& operator>>(sf::Packet& packet, StructureType& info) {
    return packet >> (int&)info;
}

inline sf::Packet& operator<<(sf::Packet& packet, const Structure& info) {
    return packet << info.type << info.x << info.y << info.size << info.owner;
}

inline sf::Packet& operator>>(sf::Packet& packet, Structure& info) {
    return packet >> info.type >> info.x >> info.y >> info.size >> info.owner;
}
