#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <cmath>
#include <cstdio>
#include <algorithm>
#include "ScreenManager.h"
#include "UIHelpers.h"
#include "UserManager.h"
#include "LevelSelectScreen.h"
#include "Tablero.h"
#include "ScoreScreen.h"

/*  GameScreen.h
 *  Pantalla principal del juego
 *
 *  CONTROLES:
 *  Click izquierdo - revelar celda
 *  Click derecho - poner/quitar bandera
 *  R - reiniciar partida (mismo nivel)
 *  ESC - vuelve a seleccion de nivel
*/

// colores estandar de los numeros 1-8
static const sf::Color COLORES_NUMEROS[9] = {
    {0,   0,   0},
    { 30, 100, 255}, // 1 azul
    { 30, 160,  30}, // 2 verde
    {220,  40,  40}, // 3 rojo
    { 10,  10, 160}, // 4 azul oscuro
    {140,  20,  20}, // 5 rojo oscuro
    { 30, 180, 180}, // 6 cyan
    { 80,  80,  80}, // 7 gris oscuro
    {110, 110, 110}, // 8 gris
};

enum class EstadoJuego {
    JUGANDO,
    GANADO,
    PERDIDO
};

class PantallaJuego {
public:

    PantallaJuego(sf::RenderWindow& ventana, const sf::Font& fuente,
                  PlayerData& jugador, LevelConfig& nivel,
                  ScoreScreen& pantallaPuntajes)
        : m_ventana(ventana), m_fuente(fuente), m_jugador(jugador),
        m_configNivel(nivel), m_pantallaPuntajes(pantallaPuntajes) {}

    ~PantallaJuego() {
        delete m_tablero;
        m_tablero = nullptr;
    }

    void onEnter() { iniciarPartida(); }

    ::GameScreen handleEvent(const sf::Event& evento) {

        if (evento.type == sf::Event::KeyReleased) {
            if (evento.key.code == sf::Keyboard::Escape) {
                if (m_jugador.modoJuego == ModoJuego::COMPETITIVO)
                    reiniciarSesionCompetitiva();
                return ::GameScreen::LEVEL_SELECT;
            }
            if (evento.key.code == sf::Keyboard::R) {
                if (m_jugador.modoJuego == ModoJuego::COMPETITIVO)
                    reiniciarSesionCompetitiva();
                iniciarPartida();
            }
        }

        if (evento.type == sf::Event::MouseButtonReleased &&
            m_estado == EstadoJuego::JUGANDO) {
            int fila = 0, col = 0;
            if (pixelACelda(evento.mouseButton.x, evento.mouseButton.y, fila, col)) {
                if (evento.mouseButton.button == sf::Mouse::Left)
                    procesarClickIzquierdo(fila, col);
                else if (evento.mouseButton.button == sf::Mouse::Right)
                    procesarClickDerecho(fila, col);
            }
        }

        if (m_estado != EstadoJuego::JUGANDO) {
            if (m_btnReiniciar.isClicked(evento)) {
                if (m_jugador.modoJuego == ModoJuego::COMPETITIVO)
                    reiniciarSesionCompetitiva();
                iniciarPartida();
            }
            if (m_btnMenu.isClicked(evento)) {
                if (m_jugador.modoJuego == ModoJuego::COMPETITIVO)
                    reiniciarSesionCompetitiva();
                return ::GameScreen::LEVEL_SELECT;
            }
            if (m_estado == EstadoJuego::GANADO &&
                m_jugador.modoJuego == ModoJuego::COMPETITIVO &&
                m_jugador.nivelCompetitivoActual < 3) {
                if (m_btnSiguiente.isClicked(evento))
                    return ::GameScreen::LEVEL_SELECT;
            }
        }

        return ::GameScreen::GAME;
    }

    void update(float dt) {
        if (m_estado == EstadoJuego::JUGANDO && m_timerActivo)
            m_tiempoSegundos += dt;

        if (m_estado != EstadoJuego::JUGANDO) {
            m_btnReiniciar.update(m_ventana);
            m_btnMenu.update(m_ventana);
            if (m_estado == EstadoJuego::GANADO &&
                m_jugador.modoJuego == ModoJuego::COMPETITIVO &&
                m_jugador.nivelCompetitivoActual < 3)
                m_btnSiguiente.update(m_ventana);
        }
        m_tiempo += dt;
    }

    void draw() {
        const float W = static_cast<float>(m_ventana.getSize().x);
        const float H = static_cast<float>(m_ventana.getSize().y);

        m_ventana.draw(UI::makeGradientBackground(
            static_cast<unsigned>(W), static_cast<unsigned>(H),
            {4, 7, 22}, {8, 22, 62}));

        dibujarHUD();
        dibujarTablero();

        if (m_estado != EstadoJuego::JUGANDO)
            dibujarOverlay();
    }

    bool  partidaTerminada()      const { return m_estado != EstadoJuego::JUGANDO; }
    float getTiempoTranscurrido() const { return m_tiempoSegundos; }

private:

    void iniciarPartida() {
        Nivel nivelTablero;
        if      (m_configNivel.minas == 10) nivelTablero = Nivel::PRINCIPIANTE;
        else if (m_configNivel.minas == 40) nivelTablero = Nivel::INTERMEDIO;
        else                                nivelTablero = Nivel::EXPERTO;

        delete m_tablero;
        m_tablero = new Tablero(nivelTablero);

        m_estado         = EstadoJuego::JUGANDO;
        m_tiempoSegundos = 0.f;
        m_timerActivo    = false;
        m_minaExploto    = {-1, -1};

        ajustarVentana();
        calcularTamanioCelda();
        configurarBotones();
    }

    void reiniciarSesionCompetitiva() {
        m_jugador.nivelCompetitivoActual = 0;
        m_jugador.tiempoAcumulado        = 0.f;
    }

    // ajusta la ventana al tamano correcto segun el nivel
    void ajustarVentana() {
        const float HUD    = 82.f;
        const float MARGEN = 18.f;

        float celdaObj;
        if      (m_tablero->getCols() <= 8)  celdaObj = 52.f; // principiante
        else if (m_tablero->getCols() <= 16) celdaObj = 34.f; // intermedio
        else                                 celdaObj = 26.f; // experto

        float areaTableroW = m_tablero->getCols() * celdaObj;
        float areaTableroH = m_tablero->getFilas() * celdaObj;

        float W = areaTableroW + MARGEN * 2.f;
        float H = areaTableroH + HUD + MARGEN * 2.f;

        // minimos razonables
        W = std::max(W, 460.f);
        H = std::max(H, 540.f);

        unsigned int nW = static_cast<unsigned int>(std::round(W));
        unsigned int nH = static_cast<unsigned int>(std::round(H));

        if (m_ventana.getSize().x != nW || m_ventana.getSize().y != nH)
            m_ventana.setSize({nW, nH});

        // reajustar la vista
        sf::View vista(sf::FloatRect(0.f, 0.f,
                                     static_cast<float>(nW), static_cast<float>(nH)));
        m_ventana.setView(vista);
    }

    void calcularTamanioCelda() {
        const float W      = static_cast<float>(m_ventana.getSize().x);
        const float H      = static_cast<float>(m_ventana.getSize().y);
        const float MARGEN = 18.f;
        const float HUD    = 82.f;

        float anchoDisponible = W - MARGEN * 2.f;
        float altoDisponible  = H - HUD - MARGEN * 2.f;

        m_tamanioCelda = std::min(
            anchoDisponible / m_tablero->getCols(),
            altoDisponible  / m_tablero->getFilas()
            );

        float tableroW = m_tamanioCelda * m_tablero->getCols();
        float tableroH = m_tamanioCelda * m_tablero->getFilas();

        m_offsetX = (W - tableroW) / 2.f;
        m_offsetY = HUD + ((altoDisponible - tableroH) / 2.f) + MARGEN;
    }

    bool pixelACelda(int px, int py, int& fila, int& col) const {
        float fx = static_cast<float>(px) - m_offsetX;
        float fy = static_cast<float>(py) - m_offsetY;

        if (fx < 0.f || fy < 0.f) return false;

        col  = static_cast<int>(fx / m_tamanioCelda);
        fila = static_cast<int>(fy / m_tamanioCelda);

        return !m_tablero->fueraDeRango(fila, col);
    }

    void procesarClickIzquierdo(int fila, int col) {
        if (!m_timerActivo) m_timerActivo = true;

        bool pisoMina = m_tablero->revelarCelda(fila, col);

        if (pisoMina) {
            m_estado      = EstadoJuego::PERDIDO;
            m_minaExploto = {fila, col};
            m_timerActivo = false;

            if (m_jugador.modoJuego == ModoJuego::COMPETITIVO)
                reiniciarSesionCompetitiva();
        }
        else if (m_tablero->verificarVictoria()) {
            m_estado      = EstadoJuego::GANADO;
            m_timerActivo = false;
            procesarVictoria();
        }
    }

    void procesarClickDerecho(int fila, int col) {
        if (!m_timerActivo) m_timerActivo = true;
        m_tablero->toggleBandera(fila, col);
    }

    void procesarVictoria() {
        if (m_jugador.modoJuego == ModoJuego::COMPETITIVO)
            procesarVictoriaCompetitiva();
        else
            procesarVictoriaLibre();
    }

    void procesarVictoriaLibre() {
        if (!m_jugador.sesionActiva) return;

        int t = static_cast<int>(m_tiempoSegundos);

        if (m_configNivel.minas == 10) {
            if (m_jugador.mejorTiempoFacil == 0 || t < m_jugador.mejorTiempoFacil)
                m_jugador.mejorTiempoFacil = t;
        }
        else if (m_configNivel.minas == 40) {
            if (m_jugador.mejorTiempoMedio == 0 || t < m_jugador.mejorTiempoMedio)
                m_jugador.mejorTiempoMedio = t;
        }
        else {
            if (m_jugador.mejorTiempoDificil == 0 || t < m_jugador.mejorTiempoDificil)
                m_jugador.mejorTiempoDificil = t;
        }

        UserManager::guardarPuntaje(m_jugador);
        m_pantallaPuntajes.addScore(m_jugador.nombre, m_tiempoSegundos, m_configNivel.nombre);
    }

    void procesarVictoriaCompetitiva() {
        m_jugador.tiempoAcumulado += m_tiempoSegundos;
        m_jugador.nivelCompetitivoActual++;

        if (m_jugador.nivelCompetitivoActual >= 3 && m_jugador.sesionActiva) {
            int total = static_cast<int>(m_jugador.tiempoAcumulado);

            if (m_jugador.mejorTiempoCompetitivo == 0 || total < m_jugador.mejorTiempoCompetitivo)
                m_jugador.mejorTiempoCompetitivo = total;

            UserManager::guardarPuntaje(m_jugador);
            m_pantallaPuntajes.addScore(m_jugador.nombre, m_jugador.tiempoAcumulado, "Competitivo");
        }
    }

    void dibujarHUD() {
        const float W  = static_cast<float>(m_ventana.getSize().x);
        const float cx = W / 2.f;

        sf::RectangleShape hud({W, 82.f});
        hud.setFillColor({8, 15, 45, 230});
        hud.setPosition(0.f, 0.f);
        m_ventana.draw(hud);

        sf::RectangleShape sep({W, 1.f});
        sep.setPosition(0.f, 82.f);
        sep.setFillColor({40, 80, 160, 120});
        m_ventana.draw(sep);

        std::string etNivel = m_configNivel.nombre;
        if (m_jugador.modoJuego == ModoJuego::COMPETITIVO)
            etNivel += "  [COMPETITIVO - Nivel " +
                       std::to_string(m_jugador.nivelCompetitivoActual + 1) + "/3]";

        sf::Text tNivel;
        tNivel.setFont(m_fuente);
        tNivel.setString(etNivel);
        tNivel.setCharacterSize(17);
        tNivel.setFillColor({130, 195, 255});
        centrarTextoEnPunto(tNivel, cx, 18.f);
        m_ventana.draw(tNivel);

        int seg = static_cast<int>(m_tiempoSegundos);
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%02d:%02d", seg / 60, seg % 60);

        sf::Text tTimer;
        tTimer.setFont(m_fuente);
        tTimer.setString(std::string("T: ") + buf);
        tTimer.setCharacterSize(19);
        tTimer.setFillColor({255, 210, 50});
        sf::FloatRect rT = tTimer.getLocalBounds();
        tTimer.setOrigin(rT.left + rT.width, rT.top);
        tTimer.setPosition(W - 12.f, 10.f);
        m_ventana.draw(tTimer);

        sf::Text tMinas;
        tMinas.setFont(m_fuente);
        tMinas.setString("M: " + std::to_string(
                             m_tablero->getTotalMinas() - m_tablero->getBanderasColocadas()));
        tMinas.setCharacterSize(19);
        tMinas.setFillColor({255, 80, 80});
        tMinas.setPosition(12.f, 10.f);
        m_ventana.draw(tMinas);

        if (m_jugador.modoJuego == ModoJuego::COMPETITIVO) {
            sf::Text tAcum;
            tAcum.setFont(m_fuente);
            tAcum.setString("Acum: " + std::to_string(
                                static_cast<int>(m_jugador.tiempoAcumulado)) + "s");
            tAcum.setCharacterSize(13);
            tAcum.setFillColor({255, 165, 30});
            tAcum.setPosition(12.f, 38.f);
            m_ventana.draw(tAcum);
        }

        sf::Text tAyuda;
        tAyuda.setFont(m_fuente);
        tAyuda.setString("R: reiniciar | ESC: menu");
        tAyuda.setCharacterSize(11);
        tAyuda.setFillColor({60, 90, 145});
        centrarTextoEnPunto(tAyuda, cx, 52.f);
        m_ventana.draw(tAyuda);
    }

    void dibujarTablero() {
        const float GAP      = std::max(1.f, m_tamanioCelda * 0.06f);
        const float CELDA_IN = m_tamanioCelda - GAP;

        for (int i = 0; i < m_tablero->getFilas(); i++) {
            for (int j = 0; j < m_tablero->getCols(); j++) {
                Celda* c = m_tablero->getCelda(i, j);
                float x  = m_offsetX + j * m_tamanioCelda;
                float y  = m_offsetY + i * m_tamanioCelda;
                dibujarCelda(c, x, y, CELDA_IN, i, j);
            }
        }
    }

    void dibujarCelda(Celda* celda, float x, float y, float tam, int fila, int col) {
        sf::RectangleShape rect({tam, tam});
        rect.setPosition(x + m_tamanioCelda * 0.03f, y + m_tamanioCelda * 0.03f);

        bool exploto = (fila == m_minaExploto.first && col == m_minaExploto.second);

        if (!celda->getEstaRevelada()) {
            rect.setFillColor({50, 68, 115});
            rect.setOutlineThickness(1.f);
            rect.setOutlineColor({20, 35, 70});
            m_ventana.draw(rect);

            if (celda->getTieneBandera())
                dibujarTextoCelda("X", x, y, tam, {240, 70, 70});

            if (m_estado == EstadoJuego::PERDIDO &&
                celda->getEsMina() && !celda->getTieneBandera()) {
                rect.setFillColor({65, 18, 18});
                m_ventana.draw(rect);
                dibujarTextoCelda("*", x, y, tam, {210, 60, 60});
            }
        }
        else {
            if (celda->getEsMina()) {
                rect.setFillColor(exploto ? sf::Color{255,45,45} : sf::Color{110,18,18});
                m_ventana.draw(rect);
                dibujarTextoCelda("*", x, y, tam,
                                  exploto ? sf::Color::White : sf::Color{200,50,50});
            }
            else {
                rect.setFillColor({15, 24, 48});
                rect.setOutlineThickness(0.5f);
                rect.setOutlineColor({25, 42, 80, 60});
                m_ventana.draw(rect);

                if (celda->getMinasVecinas() > 0)
                    dibujarTextoCelda(std::to_string(celda->getMinasVecinas()),
                                      x, y, tam, COLORES_NUMEROS[celda->getMinasVecinas()]);
            }
        }
    }

    void dibujarTextoCelda(const std::string& txt, float cx, float cy,
                           float tam, sf::Color color) {
        sf::Text t;
        t.setFont(m_fuente);
        t.setString(txt);

        unsigned int sz = static_cast<unsigned int>(tam * 0.58f);
        sz = std::max(7u, std::min(28u, sz));

        t.setCharacterSize(sz);
        t.setStyle(sf::Text::Bold);
        t.setFillColor(color);

        sf::FloatRect r = t.getLocalBounds();
        t.setOrigin(r.left + r.width / 2.f, r.top + r.height / 2.f);
        t.setPosition(cx + tam / 2.f, cy + tam / 2.f);

        m_ventana.draw(t);
    }

    void dibujarOverlay() {
        const float W  = static_cast<float>(m_ventana.getSize().x);
        const float H  = static_cast<float>(m_ventana.getSize().y);
        const float cx = W / 2.f;
        const float cy = H / 2.f;

        sf::Uint8 alpha = static_cast<sf::Uint8>(130.f + 25.f * std::sin(m_tiempo * 2.f));

        sf::RectangleShape overlay({W, H});
        overlay.setFillColor({0, 0, 0, alpha});
        m_ventana.draw(overlay);

        bool gano = (m_estado == EstadoJuego::GANADO);

        sf::RectangleShape panel({360.f, 210.f});
        panel.setOrigin(180.f, 105.f);
        panel.setPosition(cx, cy);
        panel.setFillColor(gano ? sf::Color{8,45,18,235} : sf::Color{45,8,8,235});
        panel.setOutlineColor(gano ? sf::Color{40,200,80} : sf::Color{200,40,40});
        panel.setOutlineThickness(2.f);
        m_ventana.draw(panel);

        sf::Text tTitulo;
        tTitulo.setFont(m_fuente);
        tTitulo.setCharacterSize(44);
        tTitulo.setStyle(sf::Text::Bold);

        if (gano) {
            bool todos = (m_jugador.modoJuego == ModoJuego::COMPETITIVO &&
                          m_jugador.nivelCompetitivoActual >= 3);
            tTitulo.setString(todos ? "CAMPEON!" : "VICTORIA!");
            tTitulo.setFillColor({50, 240, 95});
        } else {
            tTitulo.setString("BOOM!");
            tTitulo.setFillColor({255, 65, 45});
        }

        centrarTextoEnPunto(tTitulo, cx, cy - 62.f);
        m_ventana.draw(tTitulo);

        int seg = static_cast<int>(m_tiempoSegundos);

        sf::Text tTiempo;
        tTiempo.setFont(m_fuente);
        tTiempo.setString("Tiempo: " + std::to_string(seg / 60) + "m " +
                          std::to_string(seg % 60) + "s");
        tTiempo.setCharacterSize(17);
        tTiempo.setFillColor({175, 215, 255});
        centrarTextoEnPunto(tTiempo, cx, cy - 14.f);
        m_ventana.draw(tTiempo);

        if (m_jugador.sesionActiva) {
            sf::Text tJug;
            tJug.setFont(m_fuente);
            tJug.setString("Jugador: " + m_jugador.nombre);
            tJug.setCharacterSize(14);
            tJug.setFillColor({95, 155, 215});
            centrarTextoEnPunto(tJug, cx, cy + 10.f);
            m_ventana.draw(tJug);
        }

        m_btnReiniciar.draw(const_cast<sf::RenderWindow&>(m_ventana));
        m_btnMenu.draw(const_cast<sf::RenderWindow&>(m_ventana));

        if (gano && m_jugador.modoJuego == ModoJuego::COMPETITIVO &&
            m_jugador.nivelCompetitivoActual < 3)
            m_btnSiguiente.draw(const_cast<sf::RenderWindow&>(m_ventana));
    }

    void configurarBotones() {
        const float cx = static_cast<float>(m_ventana.getSize().x) / 2.f;
        const float cy = static_cast<float>(m_ventana.getSize().y) / 2.f;

        m_btnReiniciar.setup(m_fuente, "Reiniciar", {cx - 95.f, cy + 58.f}, {165.f, 42.f}, 16);
        m_btnReiniciar.normalColor = {20,  65,  35, 215};
        m_btnReiniciar.hoverColor  = {32, 130,  60, 235};

        m_btnMenu.setup(m_fuente, "< Menu", {cx + 95.f, cy + 58.f}, {165.f, 42.f}, 16);
        m_btnMenu.normalColor = {20, 20, 48, 215};
        m_btnMenu.hoverColor  = {48, 48,108, 235};

        m_btnSiguiente.setup(m_fuente, "Siguiente nivel >>", {cx, cy + 58.f}, {250.f, 42.f}, 16);
        m_btnSiguiente.normalColor = {10,  60,  90, 215};
        m_btnSiguiente.hoverColor  = {20, 120, 180, 235};
    }

    void centrarTextoEnPunto(sf::Text& t, float x, float y) {
        sf::FloatRect r = t.getLocalBounds();
        t.setOrigin(r.left + r.width / 2.f, r.top + r.height / 2.f);
        t.setPosition(x, y);
    }

    sf::RenderWindow& m_ventana;
    const sf::Font&   m_fuente;
    PlayerData&       m_jugador;
    LevelConfig&      m_configNivel;
    ScoreScreen&      m_pantallaPuntajes;

    Tablero*    m_tablero        = nullptr;
    EstadoJuego m_estado         = EstadoJuego::JUGANDO;
    float       m_tamanioCelda   = 32.f;
    float       m_offsetX        = 0.f;
    float       m_offsetY        = 0.f;
    float       m_tiempoSegundos = 0.f;
    float       m_tiempo         = 0.f;
    bool        m_timerActivo    = false;

    std::pair<int,int> m_minaExploto = {-1, -1};

    Button m_btnReiniciar;
    Button m_btnMenu;
    Button m_btnSiguiente;
};
