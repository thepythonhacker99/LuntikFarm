#pragma once

#include <cstdint>
#include "Structure.h"
#include <vector>
#include <entt/entt.hpp>

struct MapInfo {
    uint16_t size = 32;
    std::vector<std::vector<entt::entity>> structures;

    MapInfo() = default;

    void init() {
        structures.resize(size, std::vector<entt::entity>(size));

        for (int i = 0; i < size; i++)
            for (int j = 0; j < size; j++)
                structures[i][j] = entt::null;
    }
};
