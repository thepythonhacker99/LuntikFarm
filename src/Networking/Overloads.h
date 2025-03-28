#pragma once

#include "SFML/Network.hpp"
#include "SFML/Network/Packet.hpp"
#include <unordered_map>
#include <entt/entity/entity.hpp>

//template <typename T>
//sf::Packet &operator<<(sf::Packet &packet, const sf::Vector2<T> &vec) {
//  return packet << vec.x << vec.y;
//}
//
//template <typename T>
//sf::Packet &operator>>(sf::Packet &packet, sf::Vector2<T> &vec) {
//  return packet >> vec.x >> vec.y;
//}

sf::Packet& operator<<(sf::Packet& packet, const entt::entity& entity);
sf::Packet& operator>>(sf::Packet& packet, entt::entity& entity);

template <typename Key, typename Value>
sf::Packet &operator>>(sf::Packet &packet,
                       std::unordered_map<Key, Value> &map) {
  uint32_t size;
  packet >> size;

  for (uint32_t i = 0; i < size; i++) {
    Key k;
    Value v;

    packet >> k >> v;

    map.emplace(k, v);
  }

  return packet;
}

template <typename Key, typename Value>
sf::Packet &operator<<(sf::Packet &packet,
                       const std::unordered_map<Key, Value> &map) {
  packet << static_cast<uint32_t>(map.size());

  for (auto &[k, v] : map) {
    packet << k << v;
  }

  return packet;
}

template <typename T>
sf::Packet &operator>>(sf::Packet &packet, std::vector<T> &vector) {
  uint32_t size;
  packet >> size;

  for (uint32_t i = 0; i < size; i++) {
    T v;
    packet >> v;

    vector.push_back(v);
  }

  return packet;
}

template <typename T>
sf::Packet &operator<<(sf::Packet &packet, const std::vector<T> &vector) {
  packet << static_cast<uint32_t>(vector.size());

  for (const T &v : vector) {
    packet << v;
  }

  return packet;
}
