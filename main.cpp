//  main.cpp  –  Buscaminas
//  Punto de entrada del programa.
//
//  RESPONSABILIDADES DE ESTE ARCHIVO:
//    1. Crear la ventana SFML
//    2. Cargar la fuente
//    3. Instanciar todas las pantallas
//    4. Ejecutar el game loop principal
//    5. Gestionar las transiciones entre pantallas
//
//  PARA CONECTAR EL TABLERO (compañero del juego):
//    Busca el case GameScreen::GAME más abajo.
//    Reemplaza el contenido de ese bloque con tu clase de juego.
//    Recibirás:  chosenLevel.rows, chosenLevel.cols, chosenLevel.mines
//                player.username  (jugador activo)
// ════════════════════════════════════════════════════════════

#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>

#include "ScreenManager.h"
#include "UserManager.h"
#include "SplashScreen.h"
#include "MainMenuScreen.h"
#include "UserAuthScreen.h"
#include "LevelSelectScreen.h"

int main() {
    // Semilla aleatoria para partículas y otros efectos
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // ── Ventana principal ─────────────────────────────────────
    sf::RenderWindow window(
        sf::VideoMode(600, 680),
        "Buscaminas",
        sf::Style::Close | sf::Style::Titlebar);
    window.setFramerateLimit(60);

    // ── Fuente ────────────────────────────────────────────────
    // Carga font.ttf desde la carpeta del ejecutable.
    // Si no existe, intenta con fuentes del sistema Windows.
    sf::Font font;
    if (!font.loadFromFile("font.ttf")) {
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf") &&
            !font.loadFromFile("C:/Windows/Fonts/calibri.ttf")) {
            return -1;  // No se encontró ninguna fuente
        }
    }

    // ── Estado global compartido ──────────────────────────────
    // Estas variables se pasan por referencia a cada pantalla
    // para que todas accedan a los mismos datos.
    PlayerData  player;                   // datos del jugador activo
    LevelConfig chosenLevel = LEVELS[0];  // nivel que eligió el jugador
    GameScreen  current     = GameScreen::SPLASH;

    // ── Instanciar pantallas ──────────────────────────────────
    // Cada pantalla recibe referencias a window, font y player.
    SplashScreen      splash  (window, font);
    MainMenuScreen    menu    (window, font, player);
    UserAuthScreen    auth    (window, font, player);
    LevelSelectScreen levelSel(window, font, player, chosenLevel);

    // Activar la primera pantalla
    splash.onEnter();

    // ── Game loop ─────────────────────────────────────────────
    sf::Clock clock;

    while (window.isOpen()) {
        // Delta time: tiempo entre frames (máx 50ms para evitar saltos)
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;

        // ── Procesar eventos ──────────────────────────────────
        sf::Event event;
        GameScreen next = current;  // pantalla destino tras el evento

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            switch (current) {
            case GameScreen::SPLASH:
                splash.handleEvent(event);
                break;
            case GameScreen::MAIN_MENU:
                next = menu.handleEvent(event);
                break;
            case GameScreen::USER_AUTH:
                next = auth.handleEvent(event);
                break;
            case GameScreen::LEVEL_SELECT:
                next = levelSel.handleEvent(event);
                break;
            default: break;
            }
        }

        // ── Actualizar lógica ─────────────────────────────────
        switch (current) {
        case GameScreen::SPLASH:
            next = splash.update(dt);  // devuelve MAIN_MENU al terminar
            break;
        case GameScreen::MAIN_MENU:
            menu.update(dt);
            break;
        case GameScreen::USER_AUTH:
            auth.update(dt);
            break;
        case GameScreen::LEVEL_SELECT:
            levelSel.update(dt);
            break;
        default: break;
        }

        // ── Transición de pantalla ────────────────────────────
        // Si hubo cambio de pantalla, llamar a onEnter() de la nueva.
        if (next != current) {
            current = next;
            switch (current) {
            case GameScreen::MAIN_MENU:
                menu.onEnter();
                break;
            case GameScreen::USER_AUTH:
                auth.onEnter();
                break;
            case GameScreen::LEVEL_SELECT:
                levelSel.onEnter();
                break;

            // ── CONEXIÓN CON EL TABLERO ───────────────────
            // Cuando se llega aquí, chosenLevel tiene el nivel
            // elegido y player tiene los datos del jugador.
            //
            // El compañero que implemente el juego debe:
            //   1. Crear su clase/función de tablero aquí
            //   2. Pasarle chosenLevel.rows, .cols, .mines
            //   3. Al terminar la partida, guardar el puntaje:
            //        player.bestScoreEasy = tiempo; // si fue nivel 1
            //        UserManager::saveScore(player);
            //   4. Regresar para que el flujo vuelva al menú
            case GameScreen::GAME:
                // ← AQUÍ VA EL TABLERO DE BUSCAMINAS ←
                // (reemplazar este bloque con el código del juego)
                current = GameScreen::MAIN_MENU;
                menu.onEnter();
                break;

            default: break;
            }
        }

        // ── Dibujar ───────────────────────────────────────────
        window.clear();
        switch (current) {
        case GameScreen::SPLASH:       splash.draw();   break;
        case GameScreen::MAIN_MENU:    menu.draw();     break;
        case GameScreen::USER_AUTH:    auth.draw();     break;
        case GameScreen::LEVEL_SELECT: levelSel.draw(); break;
        default: break;
        }
        window.display();
    }

    return 0;
}
