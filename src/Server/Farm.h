#pragma once

enum FarmState {
    GROWING,
    HARVEST
};

struct Farm {
    FarmState state = GROWING;
    int time = 0;
    int growTime = 10 * 20;
};

inline sf::Packet& operator<<(sf::Packet& packet, const Farm& info) {
    return packet << info.state << info.time << info.growTime;
}

inline sf::Packet& operator>>(sf::Packet& packet, Farm& info) {
    return packet >> (int&)info.state >> info.time >> info.growTime;
}
