/* main.cpp de Buscaminas
 * Punto de entrada del programa.
 *
 *  Responsabilidades:
 *  1) Crear la ventana SFML
 *  2) Cargar la fuente
 *  3) Instanciar todas las pantallas
 *  4) Ejecutar el game loop principal
 *  5) Gestionar las transiciones entre pantallas
 *
 *  CAMBIOS PENDIENTES
 *  1) Ampliar la ventana
*/

#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>

#include "ScreenManager.h"
#include "UserManager.h"
#include "SplashScreen.h"
#include "MainMenuScreen.h"
#include "UserAuthScreen.h"
#include "LevelSelectScreen.h"
#include "GameScreen.h"

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Ventana principal
    sf::RenderWindow window(
        sf::VideoMode(600, 680),
        "Buscaminas",
        sf::Style::Close | sf::Style::Titlebar);
    window.setFramerateLimit(60);

    // Fuentes
    sf::Font font;
    if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        if (!font.loadFromFile("C:/Windows/Fonts/calibri.ttf")) {
            if (!font.loadFromFile("C:/Windows/Fonts/tahoma.ttf")) {
                return -1;
            }
        }
    }

    // Estado global compartido
    PlayerData  player;
    LevelConfig chosenLevel = LEVELS[0];
    GameScreen  current = GameScreen::SPLASH;

    // Instanciar pantallas
    SplashScreen splash(window, font);
    MainMenuScreen menu(window, font, player);
    UserAuthScreen auth(window, font, player);
    LevelSelectScreen levelSel(window, font, player, chosenLevel);
    PantallaJuego game(window, font, player, chosenLevel);

    splash.onEnter();

    // Game loop
    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        if (dt > 0.05f) {
            dt = 0.05f;
        }

        sf::Event event;
        GameScreen next = current;

        // Procesar eventos
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            switch (current) {
                case GameScreen::SPLASH: {
                    splash.handleEvent(event);
                    break;
                }
                case GameScreen::MAIN_MENU: {
                    next = menu.handleEvent(event);
                    break;
                }
                case GameScreen::USER_AUTH: {
                    next = auth.handleEvent(event);
                    break;
                }
                case GameScreen::LEVEL_SELECT: {
                    next = levelSel.handleEvent(event);
                    break;
                }
                case GameScreen::GAME: {
                    next = game.handleEvent(event);
                    break;
                }
                default: {
                    break;
                }
            }
        }

        // Actualizar lógica
        switch (current) {
            case GameScreen::SPLASH: {
                next = splash.update(dt);
                break;
            }
            case GameScreen::MAIN_MENU: {
                menu.update(dt);
                break;
            }
            case GameScreen::USER_AUTH: {
                auth.update(dt);
                break;
            }
            case GameScreen::LEVEL_SELECT: {
                levelSel.update(dt);
                break;
            }
            case GameScreen::GAME: {
                game.update(dt);
                break;
            }
            default: {
                break;
            }
        }

        // Transicion de pantalla
        if (next != current) {
            current = next;
            switch (current) {
                case GameScreen::MAIN_MENU: {
                    menu.onEnter();
                    break;
                }
                case GameScreen::USER_AUTH: {
                    auth.onEnter();
                    break;
                }
                case GameScreen::LEVEL_SELECT: {
                    levelSel.onEnter();
                    break;
                }
                case GameScreen::GAME: {
                    game.onEnter();
                    break;
                }
                default: {
                    break;
                }
            }
        }

        // Dibujar
        window.clear();
        switch (current) {
            case GameScreen::SPLASH: {
                splash.draw();
                break;
            }
            case GameScreen::MAIN_MENU: {
                menu.draw();
                break;
            }
            case GameScreen::USER_AUTH: {
                auth.draw();
                break;
            }
            case GameScreen::LEVEL_SELECT: {
                levelSel.draw();
                break;
            }
            case GameScreen::GAME: {
                game.draw();
                break;
            }
            default: {
                break;
            }
        }
        window.display();
    }

    return 0;
}
