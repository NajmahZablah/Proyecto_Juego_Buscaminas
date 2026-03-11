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
#include <SFML/Audio.hpp>       // << NUEVO: para música de fondo
#include <cstdlib>
#include <ctime>

#include "ScreenManager.h"
#include "UserManager.h"
#include "SplashScreen.h"
#include "MainMenuScreen.h"
#include "UserAuthScreen.h"
#include "LevelSelectScreen.h"
#include "GameScreen.h"

// << NUEVO: Helper para cambiar música sin cortes
//    Detiene la música actual y reproduce la nueva solo si es diferente
void switchMusic(sf::Music& music, const std::string& newFile, const std::string& currentFile) {
    if (currentFile == newFile) return;   // ya está sonando, no interrumpir
    music.stop();
    if (music.openFromFile(newFile)) {
        music.setLoop(true);
        music.setVolume(45.f);
        music.play();
    }
}

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

    // << NUEVO: Rutas de música (pon los .wav en la carpeta assets/ junto al .exe)
    const std::string MUSIC_SCREENS = "assets/MusicaScreens.wav";  // splash, menú, auth, level select
    const std::string MUSIC_GAME    = "assets/MusicaGame.wav";     // durante el juego
    // const std::string MUSIC_WIN  = "assets/WinSound.wav";       // (úsalo cuando el jugador gana)

    // << NUEVO: Objeto de música global (uno solo, se cambia según pantalla)
    sf::Music music;
    std::string currentMusicFile = "";

    // Arranca con MusicaScreens desde el splash
    switchMusic(music, MUSIC_SCREENS, currentMusicFile);
    currentMusicFile = MUSIC_SCREENS;

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
                // << NUEVO: música de pantallas
                switchMusic(music, MUSIC_SCREENS, currentMusicFile);
                currentMusicFile = MUSIC_SCREENS;
                break;
            }
            case GameScreen::USER_AUTH: {
                auth.onEnter();
                // << NUEVO: misma música, no se interrumpe
                switchMusic(music, MUSIC_SCREENS, currentMusicFile);
                currentMusicFile = MUSIC_SCREENS;
                break;
            }
            case GameScreen::LEVEL_SELECT: {
                levelSel.onEnter();
                // << NUEVO: misma música, no se interrumpe
                switchMusic(music, MUSIC_SCREENS, currentMusicFile);
                currentMusicFile = MUSIC_SCREENS;
                break;
            }
            case GameScreen::GAME: {
                game.onEnter();
                // << NUEVO: cambia a música del juego
                switchMusic(music, MUSIC_GAME, currentMusicFile);
                currentMusicFile = MUSIC_GAME;
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
