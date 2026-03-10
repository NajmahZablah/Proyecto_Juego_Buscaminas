#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <cmath>
#include <cstdio>
#include "ScreenManager.h"
#include "UIHelpers.h"
#include "UserManager.h"
#include "LevelSelectScreen.h"
#include "Tablero.h"

// GameScreen.h
// Pantalla principal del juego Buscaminas

/* Integración con el sistema:
 * Recibe LevelConfig& para saber filas/cols/minas
 * Recibe PlayerData& para guardar puntaje al ganar
 * Devuelve GameScreen::MAIN_MENU al terminar la partida
 *
 * Controles:
 * Click izquierdo -> revelar celda
 * Click derecho -> poner/quitar bandera
 * R -> reiniciar partida (mismo nivel)
 * Escape -> volver al menú principal
 *
 * Estados del juego:
 * Jugando -> partida activa
 * Ganado -> todas las celdas seguras reveladas
 * Perdido -> el jugador pisó una mina
*/

// Colores estándar de los números 1-8
static const sf::Color NUM_COLORS[9] = {
    {0, 0, 0 },
    {30, 100, 255}, // 1) azul
    {30, 160, 30}, // 2) verde
    {220, 40, 40}, // 3) rojo
    {10, 10, 160}, // 4) azul oscuro
    {140, 20, 20}, // 5) rojo oscuro
    {30, 180, 180}, // 6) cyan
    {80, 80, 80}, // 7) gris oscuro
    {110, 110, 110}, // 8) gris
};

enum class EstadoJuego {
    JUGANDO,
    GANADO,
    PERDIDO
};

class PantallaJuego {
public:

    PantallaJuego(sf::RenderWindow& win, const sf::Font& font, PlayerData& player, LevelConfig& level)
        : m_win(win), m_font(font), m_player(player), m_levelCfg(level), m_tablero(nullptr) {}

    ~PantallaJuego() {
        delete m_tablero;
        m_tablero = nullptr;
    }

    void onEnter() {
        iniciarPartida();
    }

    ::GameScreen handleEvent(const sf::Event& e) {

        // Teclado
        if (e.type == sf::Event::KeyPressed) {
            if (e.key.code == sf::Keyboard::Escape) {
                return ::GameScreen::MAIN_MENU;
            }
            if (e.key.code == sf::Keyboard::R) {
                iniciarPartida();
            }
        }

        // Mouse sobre el tablero (solo si se esta jugando)
        if (e.type == sf::Event::MouseButtonReleased &&
            m_estado == EstadoJuego::JUGANDO) {

            int fila, col;
            if (pixelACelda(e.mouseButton.x, e.mouseButton.y, fila, col)) {
                if (e.mouseButton.button == sf::Mouse::Left) {
                    procesarClickIzquierdo(fila, col);
                } else if (e.mouseButton.button == sf::Mouse::Right) {
                    procesarClickDerecho(fila, col);
                }
            }
        }

        // Botones del overlay (solo si la partida termino)
        if (m_estado != EstadoJuego::JUGANDO) {
            if (m_btnReiniciar.isClicked(e)) {
                iniciarPartida();
            }
            if (m_btnMenu.isClicked(e)) {
                return ::GameScreen::MAIN_MENU;
            }
        }

        return ::GameScreen::GAME;
    }

    void update(float dt) {
        if (m_estado == EstadoJuego::JUGANDO && m_timerActivo) {
            m_tiempoSegundos += dt;
        }

        if (m_estado != EstadoJuego::JUGANDO) {
            m_btnReiniciar.update(m_win);
            m_btnMenu.update(m_win);
        }

        m_time += dt;
    }

    void draw() {
        const float W = static_cast<float>(m_win.getSize().x);
        const float H = static_cast<float>(m_win.getSize().y);

        m_win.draw(UI::makeGradientBackground(
            static_cast<unsigned>(W), static_cast<unsigned>(H),
            {4, 7, 22}, {8, 22, 62}));

        dibujarHUD();
        dibujarTablero();

        if (m_estado != EstadoJuego::JUGANDO) {
            dibujarOverlay();
        }
    }

private:

    // Iniciar o reiniciar partida
    void iniciarPartida() {
        Nivel nivel;

        if (m_levelCfg.mines == 10) {
            nivel = Nivel::PRINCIPIANTE;
        } else if (m_levelCfg.mines == 40) {
            nivel = Nivel::INTERMEDIO;
        } else {
            nivel = Nivel::EXPERTO;
        }

        delete m_tablero;
        m_tablero = new Tablero(nivel);
        m_estado = EstadoJuego::JUGANDO;
        m_tiempoSegundos = 0.f;
        m_timerActivo = false;
        m_minaExploto = {-1, -1};

        ajustarVentana();
        calcularTamanioCelda();
        setupBotones();
    }

    // Redimensiona la ventana si el nivel lo requiere
    void ajustarVentana() {
        const float HUD_H = 68.f;
        const float MARGEN = 16.f;
        const float MIN_CELDA = 22.f;

        float minW = m_tablero->getCols() * MIN_CELDA + MARGEN * 2;
        float minH = m_tablero->getFilas() * MIN_CELDA + HUD_H + MARGEN * 2;

        unsigned int newW = static_cast<unsigned int>(std::max(minW, 600.f));
        unsigned int newH = static_cast<unsigned int>(std::max(minH, 680.f));

        if (m_win.getSize().x != newW || m_win.getSize().y != newH) {
            m_win.setSize({newW, newH});
        }
    }

    // Calcula el tamaño de celda y centra el tablero en la ventana
    void calcularTamanioCelda() {
        const float W = static_cast<float>(m_win.getSize().x);
        const float H = static_cast<float>(m_win.getSize().y);
        const float MARGEN = 16.f;
        const float HUD_H = 68.f;

        float celdaW = (W - MARGEN * 2) / m_tablero->getCols();
        float celdaH = (H - HUD_H - MARGEN * 2) / m_tablero->getFilas();
        m_celdaSize  = std::min(celdaW, celdaH);

        float totalW = m_celdaSize * m_tablero->getCols();
        float totalH = m_celdaSize * m_tablero->getFilas();
        m_offsetX = (W - totalW) / 2.f;
        m_offsetY = HUD_H + (H - HUD_H - totalH) / 2.f;
    }

    // Convierte pixel de pantalla a fila y columna del tablero
    bool pixelACelda(int px, int py, int& fila, int& col) const {
        float fx = static_cast<float>(px) - m_offsetX;
        float fy = static_cast<float>(py) - m_offsetY;

        if (fx < 0.f || fy < 0.f) {
            return false;
        }

        col = static_cast<int>(fx / m_celdaSize);
        fila = static_cast<int>(fy / m_celdaSize);

        return !m_tablero->fueraDeRango(fila, col);
    }

    // Lógica de clicks
    void procesarClickIzquierdo(int fila, int col) {
        if (!m_timerActivo) {
            m_timerActivo = true;
        }

        bool esMina = m_tablero->revelarCelda(fila, col);

        if (esMina) {
            m_estado = EstadoJuego::PERDIDO;
            m_minaExploto = {fila, col};
            m_timerActivo = false;
        } else if (m_tablero->verificarVictoria()) {
            m_estado = EstadoJuego::GANADO;
            m_timerActivo = false;
            guardarPuntaje();
        }
    }

    void procesarClickDerecho(int fila, int col) {
        if (!m_timerActivo) {
            m_timerActivo = true;
        }
        m_tablero->toggleBandera(fila, col);
    }

    // Guarda el puntaje si el jugador tiene sesion activa
    void guardarPuntaje() {
        if (!m_player.loggedIn) {
            return;
        }

        int t = static_cast<int>(m_tiempoSegundos);

        if (m_levelCfg.mines == 10) {
            m_player.bestScoreEasy = t;
        } else if (m_levelCfg.mines == 40) {
            m_player.bestScoreMed = t;
        } else {
            m_player.bestScoreHard = t;
        }

        UserManager::saveScore(m_player);
    }

    // HUD: nivel, timer, minas restantes, teclas de ayuda
    void dibujarHUD() {
        const float W = static_cast<float>(m_win.getSize().x);
        const float cx = W / 2.f;

        // Fondo del HUD
        sf::RectangleShape hudBg({W, 62.f});
        hudBg.setFillColor({8, 15, 45, 230});
        hudBg.setPosition(0.f, 0.f);
        m_win.draw(hudBg);

        // Separador inferior
        sf::RectangleShape sep({W, 1.f});
        sep.setPosition(0.f, 62.f);
        sep.setFillColor({40, 80, 160, 120});
        m_win.draw(sep);

        // Nombre del nivel (centrado)
        sf::Text lblNivel;
        lblNivel.setFont(m_font);
        lblNivel.setString(m_levelCfg.name);
        lblNivel.setCharacterSize(17);
        lblNivel.setFillColor({130, 195, 255});
        sf::FloatRect r = lblNivel.getLocalBounds();
        lblNivel.setOrigin(r.left + r.width / 2.f, r.top + r.height / 2.f);
        lblNivel.setPosition(cx, 16.f);
        m_win.draw(lblNivel);

        // Timer (derecha)
        int seg = static_cast<int>(m_tiempoSegundos);
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%02d:%02d", seg / 60, seg % 60);

        sf::Text lblTimer;
        lblTimer.setFont(m_font);
        lblTimer.setString(std::string("T: ") + buf);
        lblTimer.setCharacterSize(19);
        lblTimer.setFillColor({255, 210, 50});
        sf::FloatRect rt = lblTimer.getLocalBounds();
        lblTimer.setOrigin(rt.left + rt.width, rt.top);
        lblTimer.setPosition(W - 10.f, 8.f);
        m_win.draw(lblTimer);

        // Contador de minas (izquierda)
        int restantes = m_tablero->getTotalMinas() - m_tablero->getBanderasColocadas();

        sf::Text lblMinas;
        lblMinas.setFont(m_font);
        lblMinas.setString("M: " + std::to_string(restantes));
        lblMinas.setCharacterSize(19);
        lblMinas.setFillColor({255, 80, 80});
        lblMinas.setPosition(10.f, 8.f);
        m_win.draw(lblMinas);

        // Teclas de ayuda — parte baja del HUD
        sf::Text lblHelp;
        lblHelp.setFont(m_font);
        lblHelp.setString("R: reiniciar   ESC: menu");
        lblHelp.setCharacterSize(11);
        lblHelp.setFillColor({60, 90, 145});
        sf::FloatRect rh = lblHelp.getLocalBounds();
        lblHelp.setOrigin(rh.left + rh.width / 2.f, rh.top);
        lblHelp.setPosition(cx, 42.f);
        m_win.draw(lblHelp);
    }

    // Dibuja todas las celdas del tablero
    void dibujarTablero() {
        const float GAP      = std::max(1.f, m_celdaSize * 0.06f);
        const float CELDA_IN = m_celdaSize - GAP;

        for (int i = 0; i < m_tablero->getFilas(); i++) {
            for (int j = 0; j < m_tablero->getCols(); j++) {
                Celda* c = m_tablero->getCelda(i, j);
                float  x = m_offsetX + j * m_celdaSize;
                float  y = m_offsetY + i * m_celdaSize;
                dibujarCelda(c, x, y, CELDA_IN, i, j);
            }
        }
    }

    // Dibuja una sola celda según su estado
    void dibujarCelda(Celda* c, float x, float y, float sz, int fila, int col) {
        sf::RectangleShape rect({sz, sz});
        rect.setPosition(x + m_celdaSize * 0.03f, y + m_celdaSize * 0.03f);

        bool exploto = (fila == m_minaExploto.first && col == m_minaExploto.second);

        if (!c->getEstaRevelada()) {
            // Celda tapada
            rect.setFillColor({50, 68, 115});
            rect.setOutlineThickness(1.f);
            rect.setOutlineColor({20, 35, 70});
            m_win.draw(rect);

            if (c->getTieneBandera()) {
                dibujarTexto("F", x, y, sz, {240, 70, 70});
            }

            // En modo perdido, revelar minas sin bandera
            if (m_estado == EstadoJuego::PERDIDO &&
                c->getEsMina() && !c->getTieneBandera()) {
                rect.setFillColor({65, 18, 18});
                m_win.draw(rect);
                dibujarTexto("*", x, y, sz, {210, 60, 60});
            }

        } else {
            if (c->getEsMina()) {
                // Mina revelada
                if (exploto) {
                    rect.setFillColor({255, 45, 45});
                } else {
                    rect.setFillColor({110, 18, 18});
                }
                m_win.draw(rect);

                if (exploto) {
                    dibujarTexto("*", x, y, sz, sf::Color::White);
                } else {
                    dibujarTexto("*", x, y, sz, {200, 50, 50});
                }

            } else {
                // Celda segura revelada
                rect.setFillColor({15, 24, 48});
                rect.setOutlineThickness(0.5f);
                rect.setOutlineColor({25, 42, 80, 60});
                m_win.draw(rect);

                if (c->getMinasVecinas() > 0) {
                    dibujarTexto(std::to_string(c->getMinasVecinas()), x, y, sz, NUM_COLORS[c->getMinasVecinas()]);
                }
            }
        }
    }

    // Dibuja texto centrado dentro de una celda
    void dibujarTexto(const std::string& str, float cx, float cy, float sz, sf::Color color) {
        sf::Text t;
        t.setFont(m_font);
        t.setString(str);

        unsigned int charSz = static_cast<unsigned int>(sz * 0.58f);

        if (charSz < 7) {
            charSz = 7;
        }

        if (charSz > 28) {
            charSz = 28;
        }

        t.setCharacterSize(charSz);
        t.setStyle(sf::Text::Bold);
        t.setFillColor(color);
        sf::FloatRect r = t.getLocalBounds();
        t.setOrigin(r.left + r.width / 2.f, r.top + r.height / 2.f);
        t.setPosition(cx + sz / 2.f, cy + sz / 2.f);
        m_win.draw(t);
    }

    // Overlay de victoria o derrota
    void dibujarOverlay() {
        const float W = static_cast<float>(m_win.getSize().x);
        const float H = static_cast<float>(m_win.getSize().y);
        const float cx = W / 2.f;
        const float cy = H / 2.f;

        // Fondo semitransparente pulsante
        sf::Uint8 a = static_cast<sf::Uint8>(130.f + 25.f * std::sin(m_time * 2.f));
        sf::RectangleShape overlay({W, H});
        overlay.setFillColor({0, 0, 0, a});
        m_win.draw(overlay);

        // Panel central
        bool gano = (m_estado == EstadoJuego::GANADO);

        sf::RectangleShape panel({360.f, 210.f});
        panel.setOrigin(180.f, 105.f);
        panel.setPosition(cx, cy);

        if (gano) {
            panel.setFillColor({8, 45, 18, 235});
            panel.setOutlineColor({40, 200, 80});
        } else {
            panel.setFillColor({45, 8, 8, 235});
            panel.setOutlineColor({200, 40, 40});
        }
        panel.setOutlineThickness(2.f);
        m_win.draw(panel);

        // Titulo del resultado
        sf::Text titulo;
        titulo.setFont(m_font);
        titulo.setCharacterSize(44);
        titulo.setStyle(sf::Text::Bold);

        if (gano) {
            titulo.setString("VICTORIA!");
            titulo.setFillColor({50, 240, 95});
        } else {
            titulo.setString("BOOM!");
            titulo.setFillColor({255, 65, 45});
        }

        sf::FloatRect rt = titulo.getLocalBounds();
        titulo.setOrigin(rt.left + rt.width / 2.f, rt.top + rt.height / 2.f);
        titulo.setPosition(cx, cy - 62.f);
        m_win.draw(titulo);

        // Tiempo de la partida
        int seg = static_cast<int>(m_tiempoSegundos);

        sf::Text lblT;
        lblT.setFont(m_font);
        lblT.setString("Tiempo: " + std::to_string(seg / 60) + "m " + std::to_string(seg % 60) + "s");
        lblT.setCharacterSize(17);
        lblT.setFillColor({175, 215, 255});
        sf::FloatRect rr = lblT.getLocalBounds();
        lblT.setOrigin(rr.left + rr.width / 2.f, rr.top + rr.height / 2.f);
        lblT.setPosition(cx, cy - 14.f);
        m_win.draw(lblT);

        // Nombre del jugador (si tiene sesion)
        if (m_player.loggedIn) {
            sf::Text lblJ;
            lblJ.setFont(m_font);
            lblJ.setString("Jugador: " + m_player.username);
            lblJ.setCharacterSize(14);
            lblJ.setFillColor({95, 155, 215});
            sf::FloatRect rj = lblJ.getLocalBounds();
            lblJ.setOrigin(rj.left + rj.width / 2.f, rj.top + rj.height / 2.f);
            lblJ.setPosition(cx, cy + 10.f);
            m_win.draw(lblJ);
        }

        // Botones de resultado
        m_btnReiniciar.draw(const_cast<sf::RenderWindow&>(m_win));
        m_btnMenu.draw(const_cast<sf::RenderWindow&>(m_win));
    }

    // Configura los botones del overlay segun la posicion actual de la ventana
    void setupBotones() {
        const float cx = static_cast<float>(m_win.getSize().x) / 2.f;
        const float cy = static_cast<float>(m_win.getSize().y) / 2.f;

        m_btnReiniciar.setup(m_font, "Reiniciar", {cx - 95.f, cy + 58.f}, {165.f, 42.f}, 16);
        m_btnReiniciar.normalColor = {20, 65, 35, 215};
        m_btnReiniciar.hoverColor  = {32, 130, 60, 235};

        m_btnMenu.setup(m_font, "< Menu", {cx + 95.f, cy + 58.f}, {165.f, 42.f}, 16);
        m_btnMenu.normalColor = {20, 20, 48, 215};
        m_btnMenu.hoverColor  = {48, 48, 108, 235};
    }

    // Variables miembro
    sf::RenderWindow& m_win;
    const sf::Font& m_font;
    PlayerData& m_player;
    LevelConfig& m_levelCfg;

    Tablero* m_tablero = nullptr;
    EstadoJuego m_estado = EstadoJuego::JUGANDO;

    float m_celdaSize = 32.f;
    float m_offsetX = 0.f;
    float m_offsetY = 0.f;
    float m_tiempoSegundos = 0.f;
    float m_time = 0.f;
    bool m_timerActivo = false;

    std::pair<int,int> m_minaExploto = {-1, -1};

    Button m_btnReiniciar;
    Button m_btnMenu;
};
