#pragma once

#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"
#include "SFML/Graphics/RenderWindow.hpp"

#include <unordered_map>

enum KeyState {
    KEY_UP,
    KEY_DOWN,
    KEY_PRESSED,
    KEY_RELEASED,
};

class InputManager {
public:
    void update(sf::RenderWindow& window);

    bool isDown(sf::Keyboard::Key key);
    bool isPressed(sf::Keyboard::Key key);
    bool isReleased(sf::Keyboard::Key key);
    KeyState getKey(sf::Keyboard::Key key);

    bool isDown(sf::Mouse::Button button);
    bool isPressed(sf::Mouse::Button button);
    bool isReleased(sf::Mouse::Button button);
    KeyState getKey(sf::Mouse::Button button);
private:
    static bool _isKeyPressed(sf::Keyboard::Key key, sf::RenderWindow& window);
    static bool _isKeyPressed(sf::Mouse::Button key, sf::RenderWindow& window);

    void _ensureExists(sf::Keyboard::Key key);
    void _ensureExists(sf::Mouse::Button key);

    std::unordered_map<sf::Keyboard::Key, KeyState> m_Keys;
    std::unordered_map<sf::Mouse::Button, KeyState> m_MouseKeys;
};
