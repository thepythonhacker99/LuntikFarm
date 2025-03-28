#include "Renderer.h"

Renderer::Renderer(const std::string& title) : m_Title(title) {
    m_UiView.setCenter({ 0, 0 });
    m_MainView.setCenter({ 0, 0 });
}

Renderer::~Renderer() {

}

void Renderer::init() {
    m_RenderWindow.create(sf::VideoMode({ 1920, 1080 }), m_Title, sf::Style::Default);
    if (!m_Font.openFromFile("assets/Arial.ttf")) {
        throw std::runtime_error("Failed to load font");
    }
}

void Renderer::update() {
    constexpr float VIEW_HEIGHT = 1000.f;

    sf::Vector2f viewSize = {
            static_cast<float>(m_RenderWindow.getSize().x) / static_cast<float>(m_RenderWindow.getSize().y) * VIEW_HEIGHT,
            VIEW_HEIGHT
    };

    m_UiView.setSize(viewSize);
    m_MainView.setSize(viewSize);
}


