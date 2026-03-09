#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "ScreenManager.h"
#include "UIHelpers.h"

//  LevelSelectScreen.h
//  Pantalla de selección de nivel. Muestra tres opciones:
//
//    Nivel 1 – Principiante:  8×8   con 10 minas
//    Nivel 2 – Intermedio: 16×16  con 40 minas
//    Nivel 3 – Experto: 16×30  con 99 minas
//
//  Al elegir un nivel, guarda la configuración en 'outLevel'
//  y navega a GAME para que el tablero use esos parámetros.
//
//  CÓMO CONECTAR EL TABLERO (para el compañero del juego):
//    Recibe LevelConfig& outLevel con:
//      outLevel.rows: filas del tablero
//      outLevel.cols: columnas del tablero
//      outLevel.mines: cantidad de minas
//    Úsalos en tu clase Tablero o en new para reservar memoria.


// Configuración de cada nivel
// Esta struct es la que recibe el módulo del tablero.
struct LevelConfig {
    int         rows;   // filas
    int         cols;   // columnas
    int         mines;  // cantidad de minas
    std::string name;   // nombre del nivel
};

// Tabla de niveles (índice 0,1,2 = fácil, medio, difícil)
static const LevelConfig LEVELS[3] = {
    { 8,  8,  10, "Principiante"},
    {16, 16,  40, "Intermedio"  },
    {16, 30,  99, "Experto"     }
};

class LevelSelectScreen {
public:

    // Constructor
    // outLevel es una referencia: cuando el jugador elige,
    // se escribe aquí y main.cpp lo lee para iniciar el juego.
    LevelSelectScreen(sf::RenderWindow& win, const sf::Font& font,
                      PlayerData& player, LevelConfig& outLevel)
        : m_win(win), m_font(font), m_player(player),
        m_chosen(outLevel) {}

    // onEnter
    void onEnter() {
        const float W  = static_cast<float>(m_win.getSize().x);
        const float cx = W / 2.f;

        //  TÍTULO
        m_title.setFont(m_font);
        m_title.setString("SELECCIONA NIVEL");
        m_title.setCharacterSize(46);
        m_title.setStyle(sf::Text::Bold);
        m_title.setFillColor(sf::Color::White);
        centerText(m_title, cx, 75.f);

        // msj de bienvenida
        m_welcome.setFont(m_font);
        m_welcome.setCharacterSize(18);
        m_welcome.setFillColor({130, 195, 255});
        std::string wstr = m_player.loggedIn
                               ? "Hola " + m_player.username + "! Elige tu nivel:"
                               : "Elige tu nivel:";
        m_welcome.setString(wstr);
        centerText(m_welcome, cx, 128.f);

        //  TARJETAS DE NIVEL
        //  Espaciado vertical uniforme de 95px entre cada una
        //  Y=215: Principiante (verde)
        //  Y=310: Intermedio   (amarillo)
        //  Y=405: Experto      (rojo)

        const sf::Color cardNormal[3] = {
            {22,  88,  48},   // verde oscuro
            {88,  68,  14},   // amarillo oscuro
            {100, 16,  16}    // rojo oscuro
        };
        const sf::Color cardHover[3] = {
            {40,  150, 80},   // verde claro
            {160, 120, 20},   // amarillo claro
            {180,  30, 30}    // rojo claro
        };

        for (int i = 0; i < 3; ++i) {
            std::string label = levelLabel(i);
            m_btnLevel[i].setup(m_font, label,
                                {cx, 215.f + i * 95.f}, {380.f, 78.f}, 19);
            m_btnLevel[i].normalColor = cardNormal[i];
            m_btnLevel[i].hoverColor  = cardHover[i];
        }


        //  BOTÓN VOLVER
        m_btnBack.setup(m_font, "< Menu principal",
                        {cx, 530.f}, {230.f, 44.f}, 17);
        m_btnBack.normalColor = {22, 22, 50, 190};
        m_btnBack.hoverColor  = {50, 50, 110, 230};


        //  MEJORES TIEMPOS del jugador (si tiene sesión)
        m_scores.setFont(m_font);
        m_scores.setCharacterSize(13);
        m_scores.setFillColor({100, 155, 210});
    }

    //handleEvent
    GameScreen handleEvent(const sf::Event& e) {
        for (int i = 0; i < 3; ++i) {
            if (m_btnLevel[i].isClicked(e)) {
                m_chosen = LEVELS[i];  // guardar nivel elegido
                return GameScreen::GAME;
            }
        }
        if (m_btnBack.isClicked(e)) return GameScreen::MAIN_MENU;
        return GameScreen::LEVEL_SELECT;
    }

    // update
    void update(float dt) {
        for (auto& b : m_btnLevel) b.update(m_win);
        m_btnBack.update(m_win);
        m_time += dt;
    }

    //draw
    void draw() {
        const float W  = static_cast<float>(m_win.getSize().x);
        const float H  = static_cast<float>(m_win.getSize().y);
        const float cx = W / 2.f;

        // 1. FONDO
        m_win.draw(UI::makeGradientBackground(
            static_cast<unsigned>(W), static_cast<unsigned>(H),
            {4, 7, 22}, {8, 22, 62}));

        // 2. TÍTULO
        m_win.draw(m_title);

        // 3. LINEA ESTETIC
        sf::RectangleShape sep({260.f, 1.f});
        sep.setOrigin(130.f, 0.f);
        sep.setPosition(cx, 110.f);
        sep.setFillColor({55, 100, 200, 100});
        m_win.draw(sep);

        // 4. SALUDO
        m_win.draw(m_welcome);

        // 5. TARJETAS DE NIVEL
        for (auto& b : m_btnLevel) b.draw(m_win);

        // 6. BOTÓN VOLVER
        m_btnBack.draw(m_win);

        // 7. MEJORES TIEMPOS (solo si hay sesión activa)
        if (m_player.loggedIn) {
            auto fmt = [](int v) -> std::string {
                return v > 0 ? std::to_string(v) + "s" : "--";
            };
            std::string info =
                "Tus records:  Principiante: " + fmt(m_player.bestScoreEasy) +
                "   Intermedio: "               + fmt(m_player.bestScoreMed)  +
                "   Experto: "                  + fmt(m_player.bestScoreHard);
            m_scores.setString(info);
            centerText(m_scores, cx, 600.f);
            m_win.draw(m_scores);
        }
    }

private:

    // Genera el texto de la tarjeta de un nivel
    std::string levelLabel(int i) {
        const char* num[3] = {"1", "2", "3"};
        return "[" + std::string(num[i]) + "]  " +
               LEVELS[i].name + "   " +
               std::to_string(LEVELS[i].cols) + " x " +
               std::to_string(LEVELS[i].rows) + "   |   " +
               std::to_string(LEVELS[i].mines) + " minas";
    }

    // Centra un sf::Text en (x, y)
    void centerText(sf::Text& t, float x, float y) {
        sf::FloatRect r = t.getLocalBounds();
        t.setOrigin(r.left + r.width / 2.f, r.top + r.height / 2.f);
        t.setPosition(x, y);
    }


    //  VARIABLES MIEMBRO
    sf::RenderWindow& m_win;
    const sf::Font&   m_font;
    PlayerData&       m_player;
    LevelConfig&      m_chosen;  // referencia donde se guarda el nivel elegido

    sf::Text   m_title;    // "SELECCIONA NIVEL"
    sf::Text   m_welcome;  // saludo con nombre del jugador
    sf::Text   m_scores;   // mejores tiempos del jugador

    Button     m_btnLevel[3];  // tarjetas de nivel
    Button     m_btnBack;      // < Menu principal

    float      m_time = 0.f;
};
