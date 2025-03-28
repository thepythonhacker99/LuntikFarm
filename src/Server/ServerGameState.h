#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include "Utils/Utils.h"
#include "ServerPlayerInfo.h"
#include "MapInfo.h"

#include "entt/entt.hpp"
#include "NetworkEntityMap.h"

enum GameStage {
    LOBBY,
    GAME
};

struct ServerGameState {
    std::unordered_map<ID_t, ServerPlayerInfo> players;
    GameStage gameStage = LOBBY;

    MapInfo mapInfo;
    entt::registry registry;

    std::atomic<ID_t> networkId = 0;
    NetworkEntityMap NEP;
};
