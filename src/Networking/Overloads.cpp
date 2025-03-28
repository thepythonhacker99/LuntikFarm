#include "Overloads.h"

#include <cstdint>
#include <iostream>
#include <entt/entity/entity.hpp>

sf::Packet& operator<<(sf::Packet& packet, const entt::entity& entity) {
    return packet << static_cast<uint32_t>(entity);
}

sf::Packet& operator>>(sf::Packet& packet, entt::entity& entity) {
    uint32_t entityUint32;
    packet >> entityUint32;
    entity = static_cast<entt::entity>(entityUint32);
    return packet;
}

//sf::Packet& operator<<(sf::Packet& packet, const Utils::Pos& pos) {
//    return packet << pos.x << pos.y;
//}
//
//sf::Packet& operator>>(sf::Packet& packet, Utils::Pos& pos) {
//    return packet >> pos.x >> pos.y;
//}
//
//sf::Packet& operator<<(sf::Packet& packet,
//                       const GameObjects::BlockType& type) {
//    return packet << static_cast<uint8_t>(type);
//}
//
//sf::Packet& operator>>(sf::Packet& packet,
//                       GameObjects::BlockType& type) {
//    uint8_t typeUint8;
//    packet >> typeUint8;
//    type = static_cast<GameObjects::BlockType>(typeUint8);
//    return packet;
//}
//
//sf::Packet& operator<<(sf::Packet& packet,
//                       const GameObjects::Chunk& chunk) {
//    packet << chunk.pos;
//
//    for (uint16_t i = 0; i < Settings::CHUNK_SIZE_SQUARED; i++) {
//        packet << chunk.blocks[i].type;
//    }
//
//    return packet;
//}
//
//sf::Packet& operator>>(sf::Packet& packet, GameObjects::Chunk& chunk) {
//    packet >> chunk.pos;
//
//    for (uint16_t i = 0; i < Settings::CHUNK_SIZE_SQUARED; i++) {
//        packet >> chunk.blocks[i].type;
//        chunk.blocks[i].pos =
//                Utils::Pos(uint16_t(i % Settings::CHUNK_SIZE),
//                                   uint16_t(i / Settings::CHUNK_SIZE));
//    }
//
//    return packet;
//}
//
//sf::Packet& operator<<(sf::Packet& packet,
//                       const Entities::PlayerLOG_INFO& player) {
//    return packet << player.socketHandle << player.pos;
//}
//
//sf::Packet& operator>>(sf::Packet& packet,
//                       Entities::PlayerLOG_INFO& player) {
//    return packet >> player.socketHandle >> player.pos;
//}
