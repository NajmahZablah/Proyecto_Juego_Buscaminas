#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include "ScreenManager.h"
#include "UIHelpers.h"

/*  MainMenuScreen.h
 *  El Menú principal Muestra:
 *  - Título animado
 *  - Nombre del jugador si hay sesión activa
 *  - Botones: JUGAR, PUNTAJES, SALIR
 *  - Botón Cerrar sesión debajo de SALIR (con sesión activa)
 *  - Partículas decorativas de fondo
 *
 *  JUGAR:
 *  Con sesión activa - va directo a LEVEL_SELECT
 *  Sin sesión - va a USER_AUTH
 *
 *  CERRAR SESIÓN:
 *  Resetea PlayerData y permanece en el menú principal
 *  Solo aparece cuando sesionActiva == true.
*/

struct Particle {
    sf::CircleShape forma;
    sf::Vector2f velocidad;
    float vida;
};

class MainMenuScreen {
public:

    MainMenuScreen(sf::RenderWindow& ventana, const sf::Font& fuente, PlayerData& jugador)
        : m_ventana(ventana), m_fuente(fuente), m_jugador(jugador) {}

    void onEnter() {
        const float anchoVentana = static_cast<float>(m_ventana.getSize().x);
        const float altoVentana = static_cast<float>(m_ventana.getSize().y);
        const float centroX = anchoVentana / 2.f;

        // Título
        m_titulo.setFont(m_fuente);
        m_titulo.setString("BUSCAMINAS");
        m_titulo.setCharacterSize(66);
        m_titulo.setStyle(sf::Text::Bold);
        m_titulo.setLetterSpacing(2.8f);
        m_titulo.setFillColor(sf::Color::White);
        centrarTexto(m_titulo, centroX, 108.f);

        // Info del jugador
        m_textoJugador.setFont(m_fuente);
        m_textoJugador.setCharacterSize(18);
        m_textoJugador.setFillColor({130, 195, 255});

        // Botones principales (espaciado uniforme de 80px)
        m_btnJugar.setup (m_fuente, "JUGAR", { centroX, 290.f}, {260.f, 56.f}, 23);
        m_btnPuntajes.setup(m_fuente, "PUNTAJES", {centroX, 370.f}, {260.f, 56.f}, 23);
        m_btnSalir.setup (m_fuente, "SALIR", {centroX, 450.f}, {260.f, 56.f}, 23);
        m_btnSalir.normalColor = {75, 18, 18, 220};
        m_btnSalir.hoverColor = {170, 35, 35, 240};

        // Botón cerrar sesión (debajo de SALIR, más pequeño)
        m_btnCerrarSesion.setup(m_fuente, "Cerrar sesion", {centroX, 518.f}, {180.f, 36.f}, 14);
        m_btnCerrarSesion.normalColor = {55, 15, 15, 180};
        m_btnCerrarSesion.hoverColor = {120, 28, 28, 220};

        // Créditos
        m_creditos.setFont(m_fuente);
        m_creditos.setString("Nadiesda | Najmah | Alex");
        m_creditos.setCharacterSize(13);
        m_creditos.setFillColor({70, 110, 170});
        centrarTexto(m_creditos, centroX, altoVentana - 22.f);

        generarParticulas(28);
        actualizarTextoJugador();
        m_tiempo = 0.f;
    }

    GameScreen handleEvent(const sf::Event& evento) {

        // Cerrar sesión (solo si hay sesión activa)
        if (m_jugador.sesionActiva && m_btnCerrarSesion.isClicked(evento)) {
            m_jugador = PlayerData{};
            actualizarTextoJugador();
            return GameScreen::MAIN_MENU;
        }

        if (m_btnJugar.isClicked(evento)) {
            if (m_jugador.sesionActiva) {
                return GameScreen::LEVEL_SELECT;
            }
            return GameScreen::USER_AUTH;
        }

        if (m_btnPuntajes.isClicked(evento)) {
            return GameScreen::SCORES;
        }
        if (m_btnSalir.isClicked(evento)) {
            m_ventana.close();
        }

        return GameScreen::MAIN_MENU;
    }

    void update(float dt) {
        m_btnJugar.update(m_ventana);
        m_btnPuntajes.update(m_ventana);
        m_btnSalir.update(m_ventana);
        actualizarParticulas(dt);
        actualizarTextoJugador();

        if (m_jugador.sesionActiva) {
            m_btnCerrarSesion.update(m_ventana);
        }

        m_tiempo += dt;
    }

    void draw() {
        const float anchoVentana = static_cast<float>(m_ventana.getSize().x);
        const float altoVentana = static_cast<float>(m_ventana.getSize().y);
        const float centroX = anchoVentana / 2.f;

        // Fondo degradado
        m_ventana.draw(UI::makeGradientBackground(static_cast<unsigned>(anchoVentana),
                                                  static_cast<unsigned>(altoVentana),
                                                  {4, 7, 22}, {8, 22, 62}));

        // Partículas
        for (auto& p : m_particulas) {
            m_ventana.draw(p.forma);
        }

        // Panel decorativo
        sf::RectangleShape panel({300.f, 260.f});
        panel.setOrigin(150.f, 130.f);
        panel.setPosition(centroX, 370.f);
        panel.setFillColor({255, 255, 255, 10});
        panel.setOutlineThickness(1.f);
        panel.setOutlineColor({90, 150, 255, 35});
        m_ventana.draw(panel);

        // Título con brillo pulsante
        float brillo = 200.f + 55.f * std::sin(m_tiempo * 1.8f);
        m_titulo.setFillColor({static_cast<sf::Uint8>(brillo), 245, 255});
        m_ventana.draw(m_titulo);

        // Línea separadora bajo el título
        sf::RectangleShape separador({260.f, 1.f});
        separador.setOrigin(130.f, 0.f);
        separador.setPosition(centroX, 145.f);
        separador.setFillColor({55, 100, 200, 100});
        m_ventana.draw(separador);

        // Info del jugador
        m_ventana.draw(m_textoJugador);

        // Botones principales
        m_btnJugar.draw(m_ventana);
        m_btnPuntajes.draw(m_ventana);
        m_btnSalir.draw(m_ventana);

        // Botón cerrar sesión (si hay sesión activa)
        if (m_jugador.sesionActiva) {
            m_btnCerrarSesion.draw(m_ventana);
        }

        // Créditos
        m_ventana.draw(m_creditos);
    }

private:

    void actualizarTextoJugador() {
        std::string info = m_jugador.sesionActiva ? "Jugador: " + m_jugador.nombre : "";
        m_textoJugador.setString(info);
        centrarTexto(m_textoJugador, static_cast<float>(m_ventana.getSize().x) / 2.f, 168.f);
    }

    void generarParticulas(int cantidad) {
        m_particulas.clear();
        unsigned anchoV = m_ventana.getSize().x;
        unsigned altoV = m_ventana.getSize().y;

        for (int i = 0; i < cantidad; i++) {
            Particle p;
            float radio = 3.f + static_cast<float>(std::rand() % 7);
            p.forma.setRadius(radio);
            p.forma.setOrigin(radio, radio);
            p.forma.setPosition(static_cast<float>(std::rand() % anchoV),
                                static_cast<float>(std::rand() % altoV));
            p.forma.setFillColor({190, 35, 35, static_cast<sf::Uint8>(55 + std::rand() % 75) });
            p.velocidad = {(std::rand() % 40 - 20) / 10.f, (0.4f + (std::rand() % 10) / 10.f)};
            p.vida = 5.f + static_cast<float>(std::rand() % 8);
            m_particulas.push_back(p);
        }
    }

    void actualizarParticulas(float dt) {
        unsigned anchoV = m_ventana.getSize().x;
        unsigned altoV = m_ventana.getSize().y;

        for (auto& p : m_particulas) {
            p.forma.move(p.velocidad * dt * 30.f);
            p.vida -= dt;

            if (p.forma.getPosition().y < -20.f || p.vida <= 0.f) {
                p.forma.setPosition(static_cast<float>(std::rand() % anchoV),
                                    static_cast<float>(altoV + 10));
                p.vida = 5.f + static_cast<float>(std::rand() % 8);
            }
        }
    }

    void centrarTexto(sf::Text& texto, float x, float y) {
        sf::FloatRect limites = texto.getLocalBounds();
        texto.setOrigin(limites.left + limites.width  / 2.f, limites.top  + limites.height / 2.f);
        texto.setPosition(x, y);
    }

    // Variables miembro
    sf::RenderWindow& m_ventana;
    const sf::Font& m_fuente;
    PlayerData& m_jugador;

    sf::Text m_titulo;
    sf::Text m_textoJugador;
    sf::Text m_creditos;

    Button m_btnJugar;
    Button m_btnPuntajes;
    Button m_btnSalir;
    Button m_btnCerrarSesion;

    std::vector<Particle> m_particulas;
    float m_tiempo = 0.f;
};
