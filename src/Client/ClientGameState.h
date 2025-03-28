#pragma once

#include <string>
#include "Server/ServerGameState.h"
#include "Map.h"
#include "NetworkEntityMap.h"

struct ClientGameState {
    GameStage gameStage = LOBBY;
    std::unordered_map<ID_t, ServerPlayerInfo> players;

    MapInfo mapInfo;

    entt::registry registry;
    NetworkEntityMap NEP;
};
