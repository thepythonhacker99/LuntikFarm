#pragma once

#include "SFML/Graphics.hpp"

class Renderer {
public:
    Renderer(const std::string& title);
    ~Renderer();

    sf::RenderWindow& window() {
        return m_RenderWindow;
    }

    sf::Font &font() {
        return m_Font;
    }

    void init();
    void update();

    void setViewUI() { m_RenderWindow.setView(m_UiView); }
    [[nodiscard]] sf::Vector2f uiTopLeft() const { return m_UiView.getCenter() - m_UiView.getSize() / 2.f; }
    [[nodiscard]] sf::Vector2f uiBottomRight() const { return m_UiView.getCenter() + m_UiView.getSize() / 2.f; }
    [[nodiscard]] sf::Vector2f uiTopRight() const { return m_UiView.getCenter() + sf::Vector2f(-m_UiView.getSize().x / 2.f, m_UiView.getSize().y / 2.f); }
    [[nodiscard]] sf::Vector2f uiBottomLeft() const { return m_UiView.getCenter() + sf::Vector2f(-m_UiView.getSize().x / 2.f, m_UiView.getSize().y / 2.f); }
    [[nodiscard]] sf::Vector2f uiTopCenter() const { return m_UiView.getCenter() + sf::Vector2f(0, -m_UiView.getSize().y / 2.f); }
    [[nodiscard]] sf::Vector2f uiBottomCenter() const { return m_UiView.getCenter() + sf::Vector2f(0, m_UiView.getSize().y / 2.f); }
    [[nodiscard]] sf::Vector2f uiLeftCenter() const { return m_UiView.getCenter() + sf::Vector2f(-m_UiView.getSize().x / 2.f, 0); }
    [[nodiscard]] sf::Vector2f uiRightCenter() const { return m_UiView.getCenter() + sf::Vector2f(m_UiView.getSize().x / 2.f, 0); }
    [[nodiscard]] sf::Vector2f uiCenter() const { return m_UiView.getCenter(); }

    void setViewMain() { m_RenderWindow.setView(m_MainView); }
    sf::View &viewMain() { return m_MainView; }
    sf::View &viewUI() { return m_UiView; }
private:
    const std::string &m_Title;

    sf::RenderWindow m_RenderWindow;
    sf::View m_UiView;
    sf::View m_MainView;

    sf::Font m_Font;
};
