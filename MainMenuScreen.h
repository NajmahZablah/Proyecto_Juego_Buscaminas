#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include "ScreenManager.h"
#include "UIHelpers.h"

/*MainMenuScreen.h
 * Menú principal. Muestra:
 * - Título animado
 * - Nombre del jugador si hay sesión activa
 * - Botones: JUGAR, PUNTAJES, SALIR
 * - Pelotitas voladoras estetics
*/
struct Particle {
    sf::CircleShape shape;
    sf::Vector2f    vel;
    float           life;
};

class MainMenuScreen {
public:

    MainMenuScreen(sf::RenderWindow& win, const sf::Font& font,
                   PlayerData& player)
        : m_win(win), m_font(font), m_player(player) {}

    void onEnter() {
        const float W  = static_cast<float>(m_win.getSize().x);
        const float H  = static_cast<float>(m_win.getSize().y);
        const float cx = W / 2.f;

        m_title.setFont(m_font);

        // Título "BUSCAMINAS"
        m_title.setString("BUSCAMINAS");
        m_title.setCharacterSize(66);
        m_title.setStyle(sf::Text::Bold);
        m_title.setLetterSpacing(2.8f);     // << NUEVO: espaciado entre letras
        m_title.setFillColor(sf::Color::White);
        centerText(m_title, cx, 108.f);

        // Info jugador (solo se muestra si hay sesión activa)
        m_userInfo.setFont(m_font);
        m_userInfo.setCharacterSize(18);
        m_userInfo.setFillColor({130, 195, 255});

        // Botones: espaciado uniforme de 80px
        m_btnPlay.setup  (m_font, "JUGAR",    {cx, 290.f}, {260.f, 56.f}, 23);
        m_btnScores.setup(m_font, "PUNTAJES", {cx, 370.f}, {260.f, 56.f}, 23);
        m_btnExit.setup  (m_font, "SALIR",    {cx, 450.f}, {260.f, 56.f}, 23);
        m_btnExit.normalColor = {75, 18, 18, 220};
        m_btnExit.hoverColor  = {170, 35, 35, 240};

        // Créditos
        m_credits.setFont(m_font);
        m_credits.setString("Nadiesda | Najmah | Alex");
        m_credits.setCharacterSize(13);
        m_credits.setFillColor({70, 110, 170});
        centerText(m_credits, cx, H - 22.f);

        spawnParticles(28);
        updateUserInfo();
        m_time = 0.f;
    }

    GameScreen handleEvent(const sf::Event& e) {
        if (m_btnPlay.isClicked(e))   return GameScreen::USER_AUTH;
        if (m_btnScores.isClicked(e)) return GameScreen::LEVEL_SELECT;
        if (m_btnExit.isClicked(e))   m_win.close();
        return GameScreen::MAIN_MENU;
    }

    void update(float dt) {
        m_btnPlay.update(m_win);
        m_btnScores.update(m_win);
        m_btnExit.update(m_win);
        updateParticles(dt);
        updateUserInfo();
        m_time += dt;
    }

    void draw() {
        const float W  = static_cast<float>(m_win.getSize().x);
        const float H  = static_cast<float>(m_win.getSize().y);
        const float cx = W / 2.f;

        // Fondo
        m_win.draw(UI::makeGradientBackground(
            static_cast<unsigned>(W), static_cast<unsigned>(H),
            {4, 7, 22}, {8, 22, 62}));

        // Partículas
        for (auto& p : m_particles) m_win.draw(p.shape);

        // Panel decorativo
        sf::RectangleShape panel({300.f, 260.f});
        panel.setOrigin(150.f, 130.f);
        panel.setPosition(cx, 370.f);
        panel.setFillColor({255, 255, 255, 10});
        panel.setOutlineThickness(1.f);
        panel.setOutlineColor({90, 150, 255, 35});
        m_win.draw(panel);

        // Título con brillo pulsante
        float glow = 200.f + 55.f * std::sin(m_time * 1.8f);
        m_title.setFillColor({static_cast<sf::Uint8>(glow), 245, 255});
        m_win.draw(m_title);

        // Separador
        sf::RectangleShape sep({260.f, 1.f});
        sep.setOrigin(130.f, 0.f);
        sep.setPosition(cx, 145.f);
        sep.setFillColor({55, 100, 200, 100});
        m_win.draw(sep);

        // Info jugador (solo visible si hay sesión)
        m_win.draw(m_userInfo);

        // Botones
        m_btnPlay.draw(m_win);
        m_btnScores.draw(m_win);
        m_btnExit.draw(m_win);

        // Créditos
        m_win.draw(m_credits);
    }

private:
    void updateUserInfo() {
        std::string info = m_player.loggedIn
                               ? "Jugador: " + m_player.username
                               : "";   // cadena vacía si no hay sesión
        m_userInfo.setString(info);
        centerText(m_userInfo,
                   static_cast<float>(m_win.getSize().x) / 2.f, 168.f);
    }

    void spawnParticles(int n) {
        m_particles.clear();
        unsigned W = m_win.getSize().x, H = m_win.getSize().y;
        for (int i = 0; i < n; ++i) {
            Particle p;
            float r = 3.f + static_cast<float>(std::rand() % 7);
            p.shape.setRadius(r);
            p.shape.setOrigin(r, r);
            p.shape.setPosition(
                static_cast<float>(std::rand() % W),
                static_cast<float>(std::rand() % H));
            p.shape.setFillColor({190, 35, 35,
                                  static_cast<sf::Uint8>(55 + std::rand() % 75)});
            p.vel  = {(std::rand() % 40 - 20) / 10.f,
                     -(0.4f + (std::rand() % 10) / 10.f)};
            p.life = 5.f + static_cast<float>(std::rand() % 8);
            m_particles.push_back(p);
        }
    }

    void updateParticles(float dt) {
        unsigned W = m_win.getSize().x, H = m_win.getSize().y;
        for (auto& p : m_particles) {
            p.shape.move(p.vel * dt * 30.f);
            p.life -= dt;
            if (p.shape.getPosition().y < -20.f || p.life <= 0.f) {
                p.shape.setPosition(
                    static_cast<float>(std::rand() % W),
                    static_cast<float>(H + 10));
                p.life = 5.f + static_cast<float>(std::rand() % 8);
            }
        }
    }

    void centerText(sf::Text& t, float x, float y) {
        sf::FloatRect r = t.getLocalBounds();
        t.setOrigin(r.left + r.width / 2.f, r.top + r.height / 2.f);
        t.setPosition(x, y);
    }

    sf::RenderWindow& m_win;
    const sf::Font&   m_font;
    PlayerData&       m_player;

    sf::Font   m_titleFont; //fuente exclusiva del título
    sf::Text   m_title, m_userInfo, m_credits;
    Button     m_btnPlay, m_btnScores, m_btnExit;
    std::vector<Particle> m_particles;
    float      m_time = 0.f;
};
