#pragma once

#include "Utils/Utils.h"
#include "SFML/Network.hpp"
#include <string>

struct ServerPlayerInfo {
    std::string name;
    ID_t id;
    bool ready;

    int gold = 0;

    [[nodiscard]] bool isReady() const {
        return !name.empty();
    }
};

inline sf::Packet& operator<<(sf::Packet& packet, const ServerPlayerInfo& info) {
    return packet << info.name << info.id << info.ready;
}

inline sf::Packet& operator>>(sf::Packet& packet, ServerPlayerInfo& info) {
    return packet >> info.name >> info.id >> info.ready;
}
