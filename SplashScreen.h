#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include "ScreenManager.h"
#include "UIHelpers.h"


/*SplashScreen.h
 * Pantalla de presentación animada al iniciar el juego.
 * Duración: 3 segundos: pasa automáticamente a MAIN_MENU.
 * Click o cualquier tecla la salta.
*/
class SplashScreen {
public:

    SplashScreen(sf::RenderWindow& win, const sf::Font& font)
        : m_win(win), m_font(font) {}

    // onEnter
    void onEnter() {
        m_elapsed = 0.f;
        const float cx = m_win.getSize().x / 2.f;
        const float cy = m_win.getSize().y / 2.f;

        // Título principal
        m_title.setFont(m_font);
        m_title.setString("BUSCAMINAS");
        m_title.setCharacterSize(72);
        m_title.setStyle(sf::Text::Bold);
        m_title.setFillColor({255, 255, 255, 0});
        sf::FloatRect tr = m_title.getLocalBounds();
        m_title.setOrigin(tr.left + tr.width / 2.f, tr.top + tr.height / 2.f);
        m_title.setPosition(cx, cy - 30.f);

        // Subtítulo
        m_sub.setFont(m_font);
        m_sub.setString("Programacion 3");
        m_sub.setCharacterSize(20);
        m_sub.setFillColor({160, 220, 255, 0});
        sf::FloatRect sr = m_sub.getLocalBounds();
        m_sub.setOrigin(sr.left + sr.width / 2.f, sr.top + sr.height / 2.f);
        m_sub.setPosition(cx, cy + 55.f);

        // Círculo decorativo (mina)
        m_mine.setRadius(30.f);
        m_mine.setOrigin(30.f, 30.f);
        m_mine.setPosition(cx, cy - 120.f);
        m_mine.setFillColor({200, 40, 40, 0});
        m_mine.setOutlineThickness(3.f);
        m_mine.setOutlineColor({255, 100, 100, 0});
    }

    // update
    // Maneja el fade-in/hold/fade-out y devuelve MAIN_MENU
    // cuando termina la animación.
    GameScreen update(float dt) {
        m_elapsed += dt;

        // Fade-in 0→1s, hold 1→2.2s, fade-out 2.2→3s
        sf::Uint8 alpha = 0;
        if      (m_elapsed < 1.0f) alpha = static_cast<sf::Uint8>(m_elapsed * 255.f);
        else if (m_elapsed < 2.2f) alpha = 255;
        else if (m_elapsed < 3.0f) alpha = static_cast<sf::Uint8>((1.f - (m_elapsed - 2.2f) / 0.8f) * 255.f);

        m_title.setFillColor({255, 255, 255, alpha});
        m_sub.setFillColor({160, 220, 255, alpha});
        m_mine.setFillColor({200, 40, 40, alpha});
        m_mine.setOutlineColor({255, 100, 100, alpha});

        // Pulso del círculo
        float s = 1.f + 0.08f * std::sin(m_elapsed * 4.f);
        m_mine.setScale(s, s);

        if (m_elapsed >= 3.0f) return GameScreen::MAIN_MENU;
        return GameScreen::SPLASH;
    }

    // Click o tecla salta el splash
    void handleEvent(const sf::Event& e) {
        if (e.type == sf::Event::KeyPressed ||
            e.type == sf::Event::MouseButtonPressed)
            m_elapsed = 3.1f;
    }

    // draw
    void draw() {
        unsigned W = m_win.getSize().x, H = m_win.getSize().y;
        m_win.draw(UI::makeGradientBackground(W, H, {5,5,20}, {10,20,60}));
        m_win.draw(m_mine);
        m_win.draw(m_title);
        m_win.draw(m_sub);
    }

private:
    sf::RenderWindow& m_win;
    const sf::Font&   m_font;
    float             m_elapsed = 0.f;
    sf::Text          m_title, m_sub;
    sf::CircleShape   m_mine;
};
