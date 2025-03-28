#pragma once

#include "SFML/System/Vector2.hpp"
#include "Utils/Utils.h"
#include "SFML/Network/Packet.hpp"

enum class SoldierType {
    Basic,
};

struct Soldier {
    ID_t owner;
    SoldierType type;

    Soldier() = default;
    Soldier(ID_t owner, SoldierType type) : owner(owner), type(type) {}
};

inline sf::Packet& operator<<(sf::Packet& packet, const Soldier& soldier) {
    return packet << soldier.owner << (int) soldier.type;
}

inline sf::Packet& operator>>(sf::Packet& packet, Soldier& soldier) {
    return packet >> soldier.owner >> (int&) soldier.type;
}
