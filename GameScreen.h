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
#include "ScoreScreen.h"

/* GameScreen.h
 * Pantalla principal del juego
 *
 * CONTROLES:
 *   Click izquierdo - revelar celda
 *   Click derecho   - poner/quitar bandera
 *   R               - reiniciar partida (mismo nivel)
 *   ESC             - vuelve a selección de nivel
 *
 * MODO COMPETITIVO:
 *   Al ganar avanza automáticamente al siguiente nivel
 *   Si pierde, reinicia la sesión competitiva desde el nivel 1
 *   Al completar los 3 niveles guarda la suma de los tiempos
 */

// Colores estándar de los números 1–8
static const sf::Color COLORES_NUMEROS[9] = {
    {0,   0,   0},
    {30,  100, 255},  // 1) azul
    {30,  160, 30},   // 2) verde
    {220, 40,  40},   // 3) rojo
    {10,  10,  160},  // 4) azul oscuro
    {140, 20,  20},   // 5) rojo oscuro
    {30,  180, 180},  // 6) cyan
    {80,  80,  80},   // 7) gris oscuro
    {110, 110, 110},  // 8) gris
};

enum class EstadoJuego {
    JUGANDO,
    GANADO,
    PERDIDO
};

class PantallaJuego {
public:
    PantallaJuego(sf::RenderWindow& ventana, const sf::Font& fuente, PlayerData& jugador,
                  LevelConfig& nivel, ScoreScreen& pantallaPuntajes)
        : m_ventana(ventana), m_fuente(fuente), m_jugador(jugador),
        m_configNivel(nivel), m_pantallaPuntajes(pantallaPuntajes) {}

    ~PantallaJuego() {
        delete m_tablero;
        m_tablero = nullptr;
    }

    void onEnter() {
        iniciarPartida();
    }

    ::GameScreen handleEvent(const sf::Event& evento) {

        // ── Teclado ──────────────────────────────────────────────
        if (evento.type == sf::Event::KeyReleased) {
            if (evento.key.code == sf::Keyboard::Escape) {
                if (m_jugador.modoJuego == ModoJuego::COMPETITIVO) {
                    reiniciarSesionCompetitiva();
                }
                return ::GameScreen::LEVEL_SELECT;
            }
            if (evento.key.code == sf::Keyboard::R) {
                if (m_jugador.modoJuego == ModoJuego::COMPETITIVO) {
                    reiniciarSesionCompetitiva();
                }
                iniciarPartida();
            }
        }

        // ── Clicks sobre el tablero (solo si se está jugando) ────
        if (evento.type == sf::Event::MouseButtonReleased &&
            m_estado == EstadoJuego::JUGANDO)
        {
            int fila = 0, col = 0;
            if (pixelACelda(evento.mouseButton.x, evento.mouseButton.y, fila, col)) {
                if (evento.mouseButton.button == sf::Mouse::Left)
                    procesarClickIzquierdo(fila, col);
                else if (evento.mouseButton.button == sf::Mouse::Right)
                    procesarClickDerecho(fila, col);
            }
        }

        // ── Botones del overlay (partida terminada) ──────────────
        if (m_estado != EstadoJuego::JUGANDO) {

            bool esCompetitivoGanandoNivel =
                (m_estado == EstadoJuego::GANADO &&
                 m_jugador.modoJuego == ModoJuego::COMPETITIVO &&
                 m_jugador.nivelCompetitivoActual < 3);

            // FIX BUG 2: En modo competitivo al ganar (sin completar los 3),
            // SOLO procesar el botón "Siguiente nivel". Los botones Reiniciar
            // y Menú no deben responder porque están ocultos/solapados.
            if (esCompetitivoGanandoNivel) {
                if (m_btnSiguiente.isClicked(evento)) {
                    // FIX BUG 1: El modo competitivo se preserva explícitamente
                    // antes de volver a LevelSelect para que onEnter() no lo
                    // resetee a LIBRE.
                    m_jugador.modoJuego = ModoJuego::COMPETITIVO;
                    return ::GameScreen::LEVEL_SELECT;
                }
                // Bloquear Reiniciar y Menú — no llamar isClicked sobre ellos
                return ::GameScreen::GAME;
            }

            // Caso normal (derrota, victoria libre, o campeón competitivo):
            // los dos botones son válidos.
            if (m_btnReiniciar.isClicked(evento)) {
                if (m_jugador.modoJuego == ModoJuego::COMPETITIVO) {
                    reiniciarSesionCompetitiva();
                }
                iniciarPartida();
            }

            if (m_btnMenu.isClicked(evento)) {
                if (m_jugador.modoJuego == ModoJuego::COMPETITIVO) {
                    reiniciarSesionCompetitiva();
                }
                return ::GameScreen::LEVEL_SELECT;
            }
        }

        return ::GameScreen::GAME;
    }

    void update(float dt) {
        if (m_estado == EstadoJuego::JUGANDO && m_timerActivo)
            m_tiempoSegundos += dt;

        if (m_estado != EstadoJuego::JUGANDO) {
            bool esCompetitivoGanandoNivel =
                (m_estado == EstadoJuego::GANADO &&
                 m_jugador.modoJuego == ModoJuego::COMPETITIVO &&
                 m_jugador.nivelCompetitivoActual < 3);

            if (esCompetitivoGanandoNivel) {
                // FIX BUG 2: Solo actualizar el botón que está visible
                m_btnSiguiente.update(m_ventana);
            } else {
                m_btnReiniciar.update(m_ventana);
                m_btnMenu.update(m_ventana);
            }
        }

        m_tiempo += dt;
    }

    void draw() {
        // Recalcular en cada frame con el tamaño real actual.
        calcularDimensionesVentana();
        recalcularLayout();

        m_ventana.draw(UI::makeGradientBackground(
            static_cast<unsigned>(m_anchoVentana),
            static_cast<unsigned>(m_altoVentana),
            {4, 7, 22}, {8, 22, 62}));

        dibujarHUD();
        dibujarTablero();

        if (m_estado != EstadoJuego::JUGANDO)
            dibujarOverlay();
    }

    bool  partidaTerminada()    const { return m_estado != EstadoJuego::JUGANDO; }
    float getTiempoTranscurrido() const { return m_tiempoSegundos; }

private:

    // ── Inicialización ───────────────────────────────────────────
    void iniciarPartida() {
        Nivel nivelTablero;
        if      (m_configNivel.minas == 10) nivelTablero = Nivel::PRINCIPIANTE;
        else if (m_configNivel.minas == 40) nivelTablero = Nivel::INTERMEDIO;
        else                                nivelTablero = Nivel::EXPERTO;

        delete m_tablero;
        m_tablero = new Tablero(nivelTablero);

        m_estado          = EstadoJuego::JUGANDO;
        m_tiempoSegundos  = 0.f;
        m_timerActivo     = false;
        m_minaExploto     = {-1, -1};

        // Calcular dimensiones y layout una vez al entrar.
        // draw() también lo recalcula cada frame para mantener sync.
        calcularDimensionesVentana();
        recalcularLayout();
        configurarBotones();
    }

    void reiniciarSesionCompetitiva() {
        m_jugador.nivelCompetitivoActual = 0;
        m_jugador.tiempoAcumulado        = 0.f;
    }

    // ── Ventana y geometría ──────────────────────────────────────

    // Siempre usa el tamaño real de la ventana física como área de juego.
    // El tamaño de celda se ajusta automáticamente en recalcularLayout().
    void calcularDimensionesVentana() {
        m_anchoVentana = static_cast<float>(m_ventana.getSize().x);
        m_altoVentana  = static_cast<float>(m_ventana.getSize().y);
    }

    // Recalcula offsets y tamaño de celda usando las dimensiones
    // almacenadas, sin consultar getSize().
    void recalcularLayout() {
        const float MARGEN   = 16.f;
        const float HUD_ALTO = 68.f;

        float celdaPorAncho = (m_anchoVentana - MARGEN * 2) / m_tablero->getCols();
        float celdaPorAlto  = (m_altoVentana - HUD_ALTO - MARGEN * 2) / m_tablero->getFilas();
        m_tamanioCelda = std::min(celdaPorAncho, celdaPorAlto);

        float tableroAncho = m_tamanioCelda * m_tablero->getCols();
        float tableroAlto  = m_tamanioCelda * m_tablero->getFilas();
        m_offsetX = (m_anchoVentana - tableroAncho) / 2.f;
        m_offsetY = HUD_ALTO + (m_altoVentana - HUD_ALTO - tableroAlto) / 2.f;
    }

    void calcularTamanioCelda() { recalcularLayout(); }

    bool pixelACelda(int px, int py, int& fila, int& col) const {
        float fx = static_cast<float>(px) - m_offsetX;
        float fy = static_cast<float>(py) - m_offsetY;
        if (fx < 0.f || fy < 0.f) return false;
        col  = static_cast<int>(fx / m_tamanioCelda);
        fila = static_cast<int>(fy / m_tamanioCelda);
        return !m_tablero->fueraDeRango(fila, col);
    }

    // ── Lógica de clicks ─────────────────────────────────────────
    void procesarClickIzquierdo(int fila, int col) {
        if (!m_timerActivo) m_timerActivo = true;

        bool pisoMina = m_tablero->revelarCelda(fila, col);
        if (pisoMina) {
            m_estado      = EstadoJuego::PERDIDO;
            m_minaExploto = {fila, col};
            m_timerActivo = false;
            if (m_jugador.modoJuego == ModoJuego::COMPETITIVO)
                reiniciarSesionCompetitiva();
        } else if (m_tablero->verificarVictoria()) {
            m_estado      = EstadoJuego::GANADO;
            m_timerActivo = false;
            procesarVictoria();
            // FIX BUG 3: Recalcular posición de botones DESPUÉS de
            // procesar victoria para que coincidan con la ventana actual
            // (sin redimensionar, solo reposicionar con las medidas vigentes).
            configurarBotones();
        }
    }

    void procesarClickDerecho(int fila, int col) {
        if (!m_timerActivo) m_timerActivo = true;
        m_tablero->toggleBandera(fila, col);
    }

    // ── Lógica de victoria ───────────────────────────────────────
    void procesarVictoria() {
        if (m_jugador.modoJuego == ModoJuego::COMPETITIVO)
            procesarVictoriaCompetitiva();
        else
            procesarVictoriaLibre();
    }

    void procesarVictoriaLibre() {
        if (!m_jugador.sesionActiva) return;

        int tiempoEntero = static_cast<int>(m_tiempoSegundos);
        if (m_configNivel.minas == 10) {
            if (m_jugador.mejorTiempoFacil == 0 || tiempoEntero < m_jugador.mejorTiempoFacil)
                m_jugador.mejorTiempoFacil = tiempoEntero;
        } else if (m_configNivel.minas == 40) {
            if (m_jugador.mejorTiempoMedio == 0 || tiempoEntero < m_jugador.mejorTiempoMedio)
                m_jugador.mejorTiempoMedio = tiempoEntero;
        } else {
            if (m_jugador.mejorTiempoDificil == 0 || tiempoEntero < m_jugador.mejorTiempoDificil)
                m_jugador.mejorTiempoDificil = tiempoEntero;
        }
        UserManager::guardarPuntaje(m_jugador);
        m_pantallaPuntajes.addScore(m_jugador.nombre, m_tiempoSegundos, m_configNivel.nombre);
    }

    void procesarVictoriaCompetitiva() {
        m_jugador.tiempoAcumulado += m_tiempoSegundos;
        m_jugador.nivelCompetitivoActual++;

        if (m_jugador.nivelCompetitivoActual >= 3) {
            int tiempoTotal = static_cast<int>(m_jugador.tiempoAcumulado);
            if (m_jugador.sesionActiva) {
                if (m_jugador.mejorTiempoCompetitivo == 0 ||
                    tiempoTotal < m_jugador.mejorTiempoCompetitivo)
                {
                    m_jugador.mejorTiempoCompetitivo = tiempoTotal;
                }
                UserManager::guardarPuntaje(m_jugador);
                m_pantallaPuntajes.addScore(
                    m_jugador.nombre, m_jugador.tiempoAcumulado, "Competitivo");
            }
        }
    }

    // ── HUD ──────────────────────────────────────────────────────
    void dibujarHUD() {
        const float anchoVentana = m_anchoVentana;
        const float centroX      = anchoVentana / 2.f;

        sf::RectangleShape fondoHUD({ anchoVentana, 62.f });
        fondoHUD.setFillColor({8, 15, 45, 230});
        fondoHUD.setPosition(0.f, 0.f);
        m_ventana.draw(fondoHUD);

        sf::RectangleShape separador({ anchoVentana, 1.f });
        separador.setPosition(0.f, 62.f);
        separador.setFillColor({40, 80, 160, 120});
        m_ventana.draw(separador);

        // Etiqueta de nivel
        std::string etiquetaNivel = m_configNivel.nombre;
        if (m_jugador.modoJuego == ModoJuego::COMPETITIVO) {
            etiquetaNivel += " [COMPETITIVO - Nivel " +
                             std::to_string(m_jugador.nivelCompetitivoActual + 1) + "/3]";
        }
        sf::Text textoNivel;
        textoNivel.setFont(m_fuente);
        textoNivel.setString(etiquetaNivel);
        textoNivel.setCharacterSize(17);
        textoNivel.setFillColor({130, 195, 255});
        centrarTextoEnPunto(textoNivel, centroX, 16.f);
        m_ventana.draw(textoNivel);

        // Timer
        int  segundos = static_cast<int>(m_tiempoSegundos);
        char bufferTiempo[16];
        std::snprintf(bufferTiempo, sizeof(bufferTiempo),
                      "%02d:%02d", segundos / 60, segundos % 60);
        sf::Text textoTimer;
        textoTimer.setFont(m_fuente);
        textoTimer.setString(std::string("T: ") + bufferTiempo);
        textoTimer.setCharacterSize(19);
        textoTimer.setFillColor({255, 210, 50});
        sf::FloatRect limitesTimer = textoTimer.getLocalBounds();
        textoTimer.setOrigin(limitesTimer.left + limitesTimer.width, limitesTimer.top);
        textoTimer.setPosition(anchoVentana - 10.f, 8.f);
        m_ventana.draw(textoTimer);

        // Contador minas
        int minasRestantes = m_tablero->getTotalMinas() - m_tablero->getBanderasColocadas();
        sf::Text textoMinas;
        textoMinas.setFont(m_fuente);
        textoMinas.setString("M: " + std::to_string(minasRestantes));
        textoMinas.setCharacterSize(19);
        textoMinas.setFillColor({255, 80, 80});
        textoMinas.setPosition(10.f, 8.f);
        m_ventana.draw(textoMinas);

        // Tiempo acumulado competitivo
        if (m_jugador.modoJuego == ModoJuego::COMPETITIVO) {
            int acumulado = static_cast<int>(m_jugador.tiempoAcumulado);
            sf::Text textoAcumulado;
            textoAcumulado.setFont(m_fuente);
            textoAcumulado.setString("Acum: " + std::to_string(acumulado) + "s");
            textoAcumulado.setCharacterSize(13);
            textoAcumulado.setFillColor({255, 165, 30});
            textoAcumulado.setPosition(10.f, 32.f);
            m_ventana.draw(textoAcumulado);
        }

        // Ayuda
        sf::Text textoAyuda;
        textoAyuda.setFont(m_fuente);
        textoAyuda.setString("R: reiniciar | ESC: menu");
        textoAyuda.setCharacterSize(11);
        textoAyuda.setFillColor({60, 90, 145});
        centrarTextoEnPunto(textoAyuda, centroX, 42.f);
        m_ventana.draw(textoAyuda);
    }

    // ── Dibujo del tablero ───────────────────────────────────────
    void dibujarTablero() {
        const float GAP     = std::max(1.f, m_tamanioCelda * 0.06f);
        const float CELDA_IN = m_tamanioCelda - GAP;

        for (int i = 0; i < m_tablero->getFilas(); i++) {
            for (int j = 0; j < m_tablero->getCols(); j++) {
                Celda* celda = m_tablero->getCelda(i, j);
                float x = m_offsetX + j * m_tamanioCelda;
                float y = m_offsetY + i * m_tamanioCelda;
                dibujarCelda(celda, x, y, CELDA_IN, i, j);
            }
        }
    }

    void dibujarCelda(Celda* celda, float x, float y, float tamanio, int fila, int col) {
        sf::RectangleShape rect({ tamanio, tamanio });
        rect.setPosition(x + m_tamanioCelda * 0.03f, y + m_tamanioCelda * 0.03f);

        bool exploto = (fila == m_minaExploto.first && col == m_minaExploto.second);

        if (!celda->getEstaRevelada()) {
            rect.setFillColor({50, 68, 115});
            rect.setOutlineThickness(1.f);
            rect.setOutlineColor({20, 35, 70});
            m_ventana.draw(rect);

            if (celda->getTieneBandera())
                dibujarTextoCelda("X", x, y, tamanio, {240, 70, 70});

            if (m_estado == EstadoJuego::PERDIDO &&
                celda->getEsMina() && !celda->getTieneBandera())
            {
                rect.setFillColor({65, 18, 18});
                m_ventana.draw(rect);
                dibujarTextoCelda("*", x, y, tamanio, {210, 60, 60});
            }
        } else {
            if (celda->getEsMina()) {
                rect.setFillColor(exploto ? sf::Color{255, 45, 45} : sf::Color{110, 18, 18});
                m_ventana.draw(rect);
                dibujarTextoCelda("*", x, y, tamanio,
                                  exploto ? sf::Color::White : sf::Color{200, 50, 50});
            } else {
                rect.setFillColor({15, 24, 48});
                rect.setOutlineThickness(0.5f);
                rect.setOutlineColor({25, 42, 80, 60});
                m_ventana.draw(rect);
                if (celda->getMinasVecinas() > 0)
                    dibujarTextoCelda(std::to_string(celda->getMinasVecinas()),
                                      x, y, tamanio,
                                      COLORES_NUMEROS[celda->getMinasVecinas()]);
            }
        }
    }

    void dibujarTextoCelda(const std::string& texto, float cx, float cy,
                           float tamanio, sf::Color color)
    {
        sf::Text t;
        t.setFont(m_fuente);
        t.setString(texto);
        unsigned int tamLetra = static_cast<unsigned int>(tamanio * 0.58f);
        if (tamLetra < 7)  tamLetra = 7;
        if (tamLetra > 28) tamLetra = 28;
        t.setCharacterSize(tamLetra);
        t.setStyle(sf::Text::Bold);
        t.setFillColor(color);
        sf::FloatRect limites = t.getLocalBounds();
        t.setOrigin(limites.left + limites.width / 2.f, limites.top + limites.height / 2.f);
        t.setPosition(cx + tamanio / 2.f, cy + tamanio / 2.f);
        m_ventana.draw(t);
    }

    // ── Overlay de resultado ─────────────────────────────────────
    void dibujarOverlay() {
        const float anchoVentana = m_anchoVentana;
        const float altoVentana  = m_altoVentana;
        const float centroX      = anchoVentana / 2.f;
        const float centroY      = altoVentana  / 2.f;

        // Fondo semitransparente pulsante
        sf::Uint8 alpha = static_cast<sf::Uint8>(130.f + 25.f * std::sin(m_tiempo * 2.f));
        sf::RectangleShape overlay({ anchoVentana, altoVentana });
        overlay.setFillColor({0, 0, 0, alpha});
        m_ventana.draw(overlay);

        bool gano = (m_estado == EstadoJuego::GANADO);

        // Panel central
        sf::RectangleShape panel({360.f, 210.f});
        panel.setOrigin(180.f, 105.f);
        panel.setPosition(centroX, centroY);
        if (gano) {
            panel.setFillColor({8, 45, 18, 235});
            panel.setOutlineColor({40, 200, 80});
        } else {
            panel.setFillColor({45, 8, 8, 235});
            panel.setOutlineColor({200, 40, 40});
        }
        panel.setOutlineThickness(2.f);
        m_ventana.draw(panel);

        // Título
        sf::Text textoTitulo;
        textoTitulo.setFont(m_fuente);
        textoTitulo.setCharacterSize(44);
        textoTitulo.setStyle(sf::Text::Bold);
        if (gano) {
            bool completoTodos = (m_jugador.modoJuego == ModoJuego::COMPETITIVO &&
                                  m_jugador.nivelCompetitivoActual >= 3);
            textoTitulo.setString(completoTodos ? "CAMPEON!" : "VICTORIA!");
            textoTitulo.setFillColor({50, 240, 95});
        } else {
            textoTitulo.setString("BOOM!");
            textoTitulo.setFillColor({255, 65, 45});
        }
        centrarTextoEnPunto(textoTitulo, centroX, centroY - 62.f);
        m_ventana.draw(textoTitulo);

        // Tiempo
        int segundos = static_cast<int>(m_tiempoSegundos);
        sf::Text textoTiempo;
        textoTiempo.setFont(m_fuente);
        textoTiempo.setString("Tiempo: " + std::to_string(segundos / 60) +
                              "m " + std::to_string(segundos % 60) + "s");
        textoTiempo.setCharacterSize(17);
        textoTiempo.setFillColor({175, 215, 255});
        centrarTextoEnPunto(textoTiempo, centroX, centroY - 14.f);
        m_ventana.draw(textoTiempo);

        // Nombre jugador
        if (m_jugador.sesionActiva) {
            sf::Text textoJugador;
            textoJugador.setFont(m_fuente);
            textoJugador.setString("Jugador: " + m_jugador.nombre);
            textoJugador.setCharacterSize(14);
            textoJugador.setFillColor({95, 155, 215});
            centrarTextoEnPunto(textoJugador, centroX, centroY + 10.f);
            m_ventana.draw(textoJugador);
        }

        // Reconfigurar botones cada frame del overlay para que las posiciones
        // siempre coincidan con el tamaño real actual de la ventana.
        configurarBotones();

        // FIX BUG 2: En competitivo ganando nivel intermedio, SOLO dibujar
        // "Siguiente nivel". En cualquier otro caso, dibujar Reiniciar y Menú.
        bool esCompetitivoGanandoNivel =
            (gano &&
             m_jugador.modoJuego == ModoJuego::COMPETITIVO &&
             m_jugador.nivelCompetitivoActual < 3);

        if (esCompetitivoGanandoNivel) {
            m_btnSiguiente.draw(const_cast<sf::RenderWindow&>(m_ventana));
        } else {
            m_btnReiniciar.draw(const_cast<sf::RenderWindow&>(m_ventana));
            m_btnMenu.draw(const_cast<sf::RenderWindow&>(m_ventana));
        }
    }

    // ── Configuración de botones ─────────────────────────────────
    void configurarBotones() {
        const float centroX = m_anchoVentana / 2.f;
        const float centroY = m_altoVentana  / 2.f;

        // Reiniciar y Menú van lado a lado
        m_btnReiniciar.setup(m_fuente, "Reiniciar",
                             {centroX - 95.f, centroY + 58.f}, {165.f, 42.f}, 16);
        m_btnReiniciar.normalColor = {20, 65, 35, 215};
        m_btnReiniciar.hoverColor  = {32, 130, 60, 235};

        m_btnMenu.setup(m_fuente, "< Menu",
                        {centroX + 95.f, centroY + 58.f}, {165.f, 42.f}, 16);
        m_btnMenu.normalColor = {20, 20, 48, 215};
        m_btnMenu.hoverColor  = {48, 48, 108, 235};

        // FIX BUG 2: "Siguiente nivel" se posiciona SOLO en el centro,
        // ocupando el espacio completo, sin solaparse con nada porque
        // los otros dos botones no se dibujan cuando este aparece.
        m_btnSiguiente.setup(m_fuente, "Siguiente nivel >>",
                             {centroX, centroY + 58.f}, {280.f, 42.f}, 16);
        m_btnSiguiente.normalColor = {10, 60, 90, 215};
        m_btnSiguiente.hoverColor  = {20, 120, 180, 235};
    }

    // ── Helper texto ─────────────────────────────────────────────
    void centrarTextoEnPunto(sf::Text& texto, float x, float y) {
        sf::FloatRect limites = texto.getLocalBounds();
        texto.setOrigin(limites.left + limites.width  / 2.f,
                        limites.top  + limites.height / 2.f);
        texto.setPosition(x, y);
    }

    // ── Variables miembro ────────────────────────────────────────
    sf::RenderWindow&  m_ventana;
    const sf::Font&    m_fuente;
    PlayerData&        m_jugador;
    LevelConfig&       m_configNivel;
    ScoreScreen&       m_pantallaPuntajes;

    Tablero*       m_tablero        = nullptr;
    EstadoJuego    m_estado         = EstadoJuego::JUGANDO;
    float          m_anchoVentana   = 1200.f; // dimensiones lógicas de juego
    float          m_altoVentana    =  700.f; // se recalculan en iniciarPartida
    float          m_tamanioCelda   = 32.f;
    float          m_offsetX        = 0.f;
    float          m_offsetY        = 0.f;
    float          m_tiempoSegundos = 0.f;
    float          m_tiempo         = 0.f;
    bool           m_timerActivo    = false;
    std::pair<int,int> m_minaExploto = {-1, -1};

    Button m_btnReiniciar;
    Button m_btnMenu;
    Button m_btnSiguiente;
};
