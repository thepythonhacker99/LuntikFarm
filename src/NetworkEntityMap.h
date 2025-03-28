#pragma once

#include "Utils/Utils.h"
#include "entt/entt.hpp"
#include <unordered_map>

struct NetworkID {
    ID_t id;
};

inline sf::Packet& operator<<(sf::Packet& packet, const NetworkID& info) {
    return packet << info.id;
}

inline sf::Packet& operator>>(sf::Packet& packet, NetworkID& info) {
    return packet >> info.id;
}

struct NetworkEntityMap {
public:
    entt::entity get(ID_t id) {
        return map[id];
    }

    void init(entt::registry &registry) {
        registry.on_construct<NetworkID>().connect<&NetworkEntityMap::add>(this);
        registry.on_destroy<NetworkID>().connect<&NetworkEntityMap::remove>(this);
    }
private:
    std::unordered_map<ID_t, entt::entity> map;

    void add(entt::registry &registry, entt::entity entity) {
        map[registry.get<NetworkID>(entity).id] = entity;
    }

    void remove(entt::registry &registry, entt::entity entity) {
        map.erase(registry.get<NetworkID>(entity).id);
    }
};
