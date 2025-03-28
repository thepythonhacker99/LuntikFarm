#include "InputManager.h"

void InputManager::update(sf::RenderWindow& window) {
    for (auto& [key, state]: m_Keys) {
        switch (state) {
            case KEY_UP:
                if (_isKeyPressed(key, window)) state = KEY_PRESSED;
                break;
            case KEY_PRESSED:
                if (_isKeyPressed(key, window)) state = KEY_DOWN;
                else state = KEY_RELEASED;
                break;
            case KEY_DOWN:
                if (!_isKeyPressed(key, window)) state = KEY_RELEASED;
                break;
            case KEY_RELEASED:
                if (_isKeyPressed(key, window)) state = KEY_PRESSED;
                else state = KEY_UP;
                break;
        }
    }

    for (auto& [key, state]: m_MouseKeys) {
        switch (state) {
            case KEY_UP:
                if (_isKeyPressed(key, window)) state = KEY_PRESSED;
                break;
            case KEY_PRESSED:
                if (_isKeyPressed(key, window)) state = KEY_DOWN;
                else state = KEY_RELEASED;
                break;
            case KEY_DOWN:
                if (!_isKeyPressed(key, window)) state = KEY_RELEASED;
                break;
            case KEY_RELEASED:
                if (_isKeyPressed(key, window)) state = KEY_PRESSED;
                else state = KEY_UP;
                break;
        }
    }
}

void InputManager::_ensureExists(sf::Keyboard::Key key) {
    if (m_Keys.find(key) == m_Keys.end()) {
        m_Keys[key] = KEY_UP;
    }
}

void InputManager::_ensureExists(sf::Mouse::Button key) {
    if (m_MouseKeys.find(key) == m_MouseKeys.end()) {
        m_MouseKeys[key] = KEY_UP;
    }
}

bool InputManager::isDown(sf::Keyboard::Key key) {
    _ensureExists(key);
    return m_Keys[key] == KEY_DOWN || m_Keys[key] == KEY_PRESSED;
}

bool InputManager::isPressed(sf::Keyboard::Key key) {
    _ensureExists(key);
    return m_Keys[key] == KEY_PRESSED;
}

bool InputManager::isReleased(sf::Keyboard::Key key) {
    _ensureExists(key);
    return m_Keys[key] == KEY_RELEASED;
}

KeyState InputManager::getKey(sf::Keyboard::Key key) {
    _ensureExists(key);
    return m_Keys[key];
}

bool InputManager::isDown(sf::Mouse::Button button) {
    _ensureExists(button);
    return m_MouseKeys[button] == KEY_DOWN || m_MouseKeys[button] == KEY_PRESSED;
}

bool InputManager::isPressed(sf::Mouse::Button button) {
    _ensureExists(button);
    return m_MouseKeys[button] == KEY_PRESSED;
}

bool InputManager::isReleased(sf::Mouse::Button button) {
    _ensureExists(button);
    return m_MouseKeys[button] == KEY_RELEASED;
}

KeyState InputManager::getKey(sf::Mouse::Button button) {
    _ensureExists(button);
    return m_MouseKeys[button];
}

bool InputManager::_isKeyPressed(const sf::Keyboard::Key key, sf::RenderWindow& window) {
    return sf::Keyboard::isKeyPressed(key) && window.hasFocus();
}

bool InputManager::_isKeyPressed(const sf::Mouse::Button key, sf::RenderWindow& window) {
    return sf::Mouse::isButtonPressed(key) && window.hasFocus();
}
