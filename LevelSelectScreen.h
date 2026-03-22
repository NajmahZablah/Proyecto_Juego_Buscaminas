#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "ScreenManager.h"
#include "UIHelpers.h"

/* LevelSelectScreen.h
 * Pantalla de selección de nivel y modo de juego
 *
 * MODOS:
 *   Libre       - el jugador elige cualquier nivel
 *   Competitivo - debe completar nivel 1 -> 2 -> 3 en orden
 *                 solo disponible si hay sesión activa
 *
 * NIVELES:
 *   Nivel 1 – Principiante: 8×8  con 10 minas
 *   Nivel 2 – Intermedio:  16×16 con 40 minas
 *   Nivel 3 – Experto:     16×30 con 99 minas
 */

struct LevelConfig {
    int filas;
    int columnas;
    int minas;
    std::string nombre;
};

static const LevelConfig NIVELES[3] = {
    {8,  8,  10, "Principiante"},
    {16, 16, 40, "Intermedio"},
    {16, 30, 99, "Experto"}
};

class LevelSelectScreen {
public:
    LevelSelectScreen(sf::RenderWindow& ventana, const sf::Font& fuente,
                      PlayerData& jugador, LevelConfig& nivelElegido)
        : m_ventana(ventana), m_fuente(fuente),
        m_jugador(jugador), m_nivelElegido(nivelElegido) {}

    void onEnter() {
        const float anchoVentana = static_cast<float>(m_ventana.getSize().x);
        const float centroX = anchoVentana / 2.f;

        // Título
        m_titulo.setFont(m_fuente);
        m_titulo.setString("SELECCIONA NIVEL");
        m_titulo.setCharacterSize(46);
        m_titulo.setStyle(sf::Text::Bold);
        m_titulo.setFillColor(sf::Color::White);
        centrarTexto(m_titulo, centroX, 68.f);

        // Mensaje de bienvenida
        m_bienvenida.setFont(m_fuente);
        m_bienvenida.setCharacterSize(18);
        m_bienvenida.setFillColor({130, 195, 255});
        std::string msg = m_jugador.sesionActiva
                              ? "Hola " + m_jugador.nombre + "! Elige tu modo y nivel:"
                              : "Elige tu modo y nivel:";
        m_bienvenida.setString(msg);
        centrarTexto(m_bienvenida, centroX, 118.f);

        configurarPestanas(centroX);
        configurarTarjetasNivel(centroX);

        // Botón volver
        m_btnVolver.setup(m_fuente, "< Menu principal", {centroX, 570.f}, {230.f, 44.f}, 17);
        m_btnVolver.normalColor = {22, 22, 50, 190};
        m_btnVolver.hoverColor  = {50, 50, 110, 230};

        // Texto de récords
        m_textoRecords.setFont(m_fuente);
        m_textoRecords.setCharacterSize(13);
        m_textoRecords.setFillColor({100, 155, 210});

        // FIX: Respetar el modo que viene del jugador en lugar de
        // resetear siempre a LIBRE. Esto permite que al volver desde
        // "Siguiente nivel" en modo competitivo, la pantalla recuerde
        // que se está en modo competitivo.
        m_modoActual = m_jugador.modoJuego;
        actualizarEstadoBotones();
    }

    GameScreen handleEvent(const sf::Event& evento) {

        // Cambio de pestaña
        if (m_btnModoLibre.isClicked(evento)) {
            m_modoActual        = ModoJuego::LIBRE;
            m_jugador.modoJuego = ModoJuego::LIBRE;
            actualizarEstadoBotones();
        }

        if (m_btnModoCompetitivo.isClicked(evento)) {
            if (m_jugador.sesionActiva) {
                m_modoActual        = ModoJuego::COMPETITIVO;
                m_jugador.modoJuego = ModoJuego::COMPETITIVO;
                // FIX: Solo resetear el progreso competitivo cuando el jugador
                // ELIGE manualmente el modo competitivo desde el botón, NO
                // cuando regresa automáticamente después de ganar un nivel.
                // El reset solo ocurre si nivelCompetitivoActual == 0 ya
                // (inicio), o si venía de modo Libre (cambio manual de modo).
                if (m_jugador.nivelCompetitivoActual == 0) {
                    m_jugador.tiempoAcumulado = 0.f;
                }
                actualizarEstadoBotones();
            } else {
                m_mostrarAvisoSesion = true;
                m_timerAviso = 2.5f;
            }
        }

        // Selección de nivel
        for (int i = 0; i < 3; i++) {
            if (m_btnNivel[i].isClicked(evento) && !m_nivelBloqueado[i]) {
                m_nivelElegido      = NIVELES[i];
                m_jugador.modoJuego = m_modoActual;
                return GameScreen::GAME;
            }
        }

        if (m_btnVolver.isClicked(evento))
            return GameScreen::MAIN_MENU;

        return GameScreen::LEVEL_SELECT;
    }

    void update(float dt) {
        for (auto& b : m_btnNivel) b.update(m_ventana);
        m_btnModoLibre.update(m_ventana);
        m_btnModoCompetitivo.update(m_ventana);
        m_btnVolver.update(m_ventana);

        if (m_timerAviso > 0.f) {
            m_timerAviso -= dt;
            if (m_timerAviso <= 0.f) m_mostrarAvisoSesion = false;
        }
        m_tiempo += dt;
    }

    void draw() {
        const float anchoVentana = static_cast<float>(m_ventana.getSize().x);
        const float altoVentana  = static_cast<float>(m_ventana.getSize().y);
        const float centroX      = anchoVentana / 2.f;

        m_ventana.draw(UI::makeGradientBackground(
            static_cast<unsigned>(anchoVentana),
            static_cast<unsigned>(altoVentana),
            {4, 7, 22}, {8, 22, 62}));

        m_ventana.draw(m_titulo);

        sf::RectangleShape sep({260.f, 1.f});
        sep.setOrigin(130.f, 0.f);
        sep.setPosition(centroX, 102.f);
        sep.setFillColor({55, 100, 200, 100});
        m_ventana.draw(sep);

        m_ventana.draw(m_bienvenida);

        dibujarPestana(m_btnModoLibre,       m_modoActual == ModoJuego::LIBRE,        {65, 180, 255});
        dibujarPestana(m_btnModoCompetitivo,  m_modoActual == ModoJuego::COMPETITIVO,  {255, 165, 30});

        sf::RectangleShape sepModo({anchoVentana * 0.55f, 1.f});
        sepModo.setOrigin(sepModo.getSize().x / 2.f, 0.f);
        sepModo.setPosition(centroX, 198.f);
        sepModo.setFillColor({40, 70, 130, 80});
        m_ventana.draw(sepModo);

        for (int i = 0; i < 3; i++) m_btnNivel[i].draw(m_ventana);
        m_btnVolver.draw(m_ventana);

        if (m_jugador.sesionActiva) {
            auto fmt = [](int s) -> std::string {
                return s > 0 ? std::to_string(s) + "s" : "--";
            };
            m_textoRecords.setString(
                "Tus records: Principiante: " + fmt(m_jugador.mejorTiempoFacil) +
                "  Intermedio: "              + fmt(m_jugador.mejorTiempoMedio) +
                "  Experto: "                 + fmt(m_jugador.mejorTiempoDificil) +
                "  Competitivo: "             + fmt(m_jugador.mejorTiempoCompetitivo));
            centrarTexto(m_textoRecords, centroX, 630.f);
            m_ventana.draw(m_textoRecords);
        }

        if (m_mostrarAvisoSesion) dibujarAvisoSesion(centroX);
    }

private:
    void configurarPestanas(float centroX) {
        m_btnModoLibre.setup(m_fuente, "Libre",
                             {centroX - 120.f, 162.f}, {200.f, 40.f}, 17);
        m_btnModoLibre.normalColor = {22, 60, 22, 200};
        m_btnModoLibre.hoverColor  = {40, 120, 40, 230};

        m_btnModoCompetitivo.setup(m_fuente, "Competitivo",
                                   {centroX + 120.f, 162.f}, {200.f, 40.f}, 17);
        m_btnModoCompetitivo.normalColor = {70, 45, 10, 200};
        m_btnModoCompetitivo.hoverColor  = {150, 100, 20, 230};
    }

    void configurarTarjetasNivel(float centroX) {
        const sf::Color colorNormal[3] = {{22,88,48},{88,68,14},{100,16,16}};
        const sf::Color colorHover[3]  = {{40,150,80},{160,120,20},{180,30,30}};

        for (int i = 0; i < 3; i++) {
            m_colorNormalOriginal[i] = colorNormal[i];
            m_colorHoverOriginal[i]  = colorHover[i];
            m_btnNivel[i].setup(m_fuente, generarEtiquetaNivel(i),
                                {centroX, 255.f + i * 95.f}, {380.f, 78.f}, 19);
            m_btnNivel[i].normalColor = colorNormal[i];
            m_btnNivel[i].hoverColor  = colorHover[i];
        }
    }

    void actualizarEstadoBotones() {
        const sf::Color GRIS = {35, 35, 45};
        if (m_modoActual == ModoJuego::LIBRE) {
            for (int i = 0; i < 3; i++) {
                m_nivelBloqueado[i]       = false;
                m_btnNivel[i].normalColor = m_colorNormalOriginal[i];
                m_btnNivel[i].hoverColor  = m_colorHoverOriginal[i];
            }
        } else {
            for (int i = 0; i < 3; i++) {
                m_nivelBloqueado[i] = (i != m_jugador.nivelCompetitivoActual);
                if (m_nivelBloqueado[i]) {
                    m_btnNivel[i].normalColor = GRIS;
                    m_btnNivel[i].hoverColor  = GRIS;
                } else {
                    m_btnNivel[i].normalColor = m_colorNormalOriginal[i];
                    m_btnNivel[i].hoverColor  = m_colorHoverOriginal[i];
                }
            }
        }
    }

    void dibujarPestana(Button& btn, bool activa, sf::Color colorLinea) {
        if (activa) {
            sf::RectangleShape sub({200.f, 3.f});
            sf::Vector2f pos = btn.shape.getPosition();
            sub.setPosition(pos.x - 100.f, pos.y + 20.f);
            sub.setFillColor(colorLinea);
            m_ventana.draw(sub);
            btn.shape.setOutlineColor({colorLinea.r, colorLinea.g, colorLinea.b, 190});
        } else {
            btn.shape.setOutlineColor({42, 60, 100, 80});
        }
        btn.draw(m_ventana);
    }

    void dibujarAvisoSesion(float centroX) {
        const float altoVentana = static_cast<float>(m_ventana.getSize().y);
        sf::Uint8 alpha = m_timerAviso > 0.5f
                              ? 255
                              : static_cast<sf::Uint8>((m_timerAviso / 0.5f) * 255.f);
        sf::Text aviso;
        aviso.setFont(m_fuente);
        aviso.setString("Debes iniciar sesion para jugar en modo Competitivo.");
        aviso.setCharacterSize(14);
        aviso.setFillColor({255, 165, 30, alpha});
        sf::FloatRect lim = aviso.getLocalBounds();
        aviso.setOrigin(lim.left + lim.width / 2.f, lim.top + lim.height / 2.f);
        aviso.setPosition(centroX, altoVentana - 40.f);
        m_ventana.draw(aviso);
    }

    void centrarTexto(sf::Text& texto, float x, float y) {
        sf::FloatRect lim = texto.getLocalBounds();
        texto.setOrigin(lim.left + lim.width / 2.f, lim.top + lim.height / 2.f);
        texto.setPosition(x, y);
    }

    std::string generarEtiquetaNivel(int i) {
        const char* nums[3] = {"1","2","3"};
        return "[" + std::string(nums[i]) + "] " +
               NIVELES[i].nombre + "  " +
               std::to_string(NIVELES[i].columnas) + " x " +
               std::to_string(NIVELES[i].filas) + "  |  " +
               std::to_string(NIVELES[i].minas) + " minas";
    }

    // Variables miembro
    sf::RenderWindow& m_ventana;
    const sf::Font&   m_fuente;
    PlayerData&       m_jugador;
    LevelConfig&      m_nivelElegido;

    sf::Text m_titulo;
    sf::Text m_bienvenida;
    sf::Text m_textoRecords;

    Button m_btnModoLibre;
    Button m_btnModoCompetitivo;
    Button m_btnNivel[3];
    Button m_btnVolver;

    sf::Color m_colorNormalOriginal[3];
    sf::Color m_colorHoverOriginal[3];

    ModoJuego m_modoActual           = ModoJuego::LIBRE;
    bool      m_nivelBloqueado[3]    = {false, false, false};
    bool      m_mostrarAvisoSesion   = false;
    float     m_timerAviso           = 0.f;
    float     m_tiempo               = 0.f;
};
