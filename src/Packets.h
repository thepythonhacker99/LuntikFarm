#pragma once

#include "Utils/Utils.h"
#include "Networking/Common.h"
#include "Server/Structure.h"
#include "Server/ServerPlayerInfo.h"
#include "Server/Farm.h"
#include "NetworkEntityMap.h"
#include "Server/Soldier.h"
#include "Server/Position.h"

enum PacketID {
    C2S_NAME_PACKET,

    S2C_LOBBY_PACKET,
    S2C_PLAYER_PACKET,
    S2C_PLAYER_QUIT_PACKET,

    C2S_READY_PACKET,
    S2C_READY_PACKET,

    S2C_START_GAME_PACKET,

    S2C_GOLD_PACKET,

    S2C_STRUCTURE_PACKET,
    S2C_STRUCTURE_DELETE_PACKET,

    S2C_FARM_PACKET,
    C2S_HARVEST_PACKET,

    C2S_PLACE_WALL_PACKET,
    C2S_PLANT_FARM_PACKET,

    S2C_SOLDIER_CREATE_PACKET,
    S2C_SOLDIER_DELETE_PACKET,
    S2C_SOLDIER_POSITION_PACKET,

    C2S_SPAWN_SOLDIER_PACKET
};

inline void registerPackets() {
    Networking::registerPacket<C2S_NAME_PACKET, std::string>();

    Networking::registerPacket<S2C_LOBBY_PACKET, std::unordered_map<ID_t, ServerPlayerInfo>>();
    Networking::registerPacket<S2C_PLAYER_PACKET, ID_t, ServerPlayerInfo>();
    Networking::registerPacket<S2C_PLAYER_QUIT_PACKET, ID_t>();

    Networking::registerPacket<C2S_READY_PACKET, bool>();
    Networking::registerPacket<S2C_READY_PACKET, ID_t, bool>();

    Networking::registerPacket<S2C_START_GAME_PACKET>();

    Networking::registerPacket<S2C_GOLD_PACKET, int>();

    Networking::registerPacket<S2C_STRUCTURE_PACKET, NetworkID, Structure>();
    Networking::registerPacket<S2C_STRUCTURE_DELETE_PACKET, NetworkID>();

    Networking::registerPacket<S2C_FARM_PACKET, NetworkID, Farm>();
    Networking::registerPacket<C2S_HARVEST_PACKET, NetworkID>();

    Networking::registerPacket<C2S_PLACE_WALL_PACKET, int, int>();
    Networking::registerPacket<C2S_PLANT_FARM_PACKET, int, int>();

    Networking::registerPacket<S2C_SOLDIER_CREATE_PACKET, NetworkID, Soldier, Position>();
    Networking::registerPacket<S2C_SOLDIER_DELETE_PACKET, NetworkID>();
    Networking::registerPacket<S2C_SOLDIER_POSITION_PACKET, NetworkID, Position>();

    Networking::registerPacket<C2S_SPAWN_SOLDIER_PACKET, float, float>();
}
