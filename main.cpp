/* main.cpp de Buscaminas
 * Punto de entrada del programa.
 *
 *  Responsabilidades:
 *  1) Crear la ventana SFML
 *  2) Cargar la fuente
 *  3) Instanciar todas las pantallas
 *  4) Ejecutar el game loop principal
 *  5) Gestionar las transiciones entre pantallas
*/

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cstdlib>
#include <ctime>

#include "ScreenManager.h"
#include "SplashScreen.h"
#include "MainMenuScreen.h"
#include "UserAuthScreen.h"
#include "LevelSelectScreen.h"   // aqui esta definido NIVELES[]
#include "GameScreen.h"
#include "ScoreScreen.h"
#include "CreditsScreen.h"

// tamano original de la ventana - se restaura al salir del juego
static const unsigned int VENTANA_W = 600;
static const unsigned int VENTANA_H = 680;

// cambia la musica sin cortes
// solo actua si el archivo nuevo es diferente al que suena
void switchMusic(sf::Music& music, const std::string& newFile,
                 const std::string& currentFile) {
    if (currentFile == newFile) return;
    music.stop();
    if (music.openFromFile(newFile)) {
        music.setLoop(true);
        music.setVolume(45.f);
        music.play();
    }
}

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // ventana principal
    sf::RenderWindow window(
        sf::VideoMode(VENTANA_W, VENTANA_H),
        "Buscaminas",
        sf::Style::Close | sf::Style::Titlebar);
    window.setFramerateLimit(60);

    // cargar fuente del sistema
    sf::Font font;
    if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        if (!font.loadFromFile("C:/Windows/Fonts/calibri.ttf")) {
            if (!font.loadFromFile("C:/Windows/Fonts/tahoma.ttf")) {
                return -1;
            }
        }
    }

    // rutas de musica
    const std::string MUSIC_SCREENS = "assets/MusicaScreens.wav";
    const std::string MUSIC_GAME    = "assets/MusicaGame.wav";

    // un solo objeto de musica para todo el juego
    sf::Music music;
    std::string currentMusicFile = "";
    switchMusic(music, MUSIC_SCREENS, currentMusicFile);
    currentMusicFile = MUSIC_SCREENS;

    // estado global compartido entre pantallas
    PlayerData  player;
    LevelConfig chosenLevel = NIVELES[0];  // NIVELES viene de LevelSelectScreen.h
    GameScreen  current = GameScreen::SPLASH;

    // instanciar todas las pantallas
    ScoreScreen       scores;
    SplashScreen      splash  (window, font);
    MainMenuScreen    menu    (window, font, player);
    UserAuthScreen    auth    (window, font, player);
    LevelSelectScreen levelSel(window, font, player, chosenLevel);
    PantallaJuego     game    (window, font, player, chosenLevel, scores);
    CreditsScreen     credits (window, font);

    splash.onEnter();

    // game loop principal
    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;

        sf::Event event;
        GameScreen next = current;

        // procesar eventos segun pantalla activa
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            switch (current) {
            case GameScreen::SPLASH:       splash.handleEvent(event);          break;
            case GameScreen::MAIN_MENU:    next = menu.handleEvent(event);     break;
            case GameScreen::USER_AUTH:    next = auth.handleEvent(event);     break;
            case GameScreen::LEVEL_SELECT: next = levelSel.handleEvent(event); break;
            case GameScreen::GAME:         next = game.handleEvent(event);     break;
            case GameScreen::SCORES:       next = scores.handleEvent(event);   break;
            case GameScreen::CREDITS:      next = credits.handleEvent(event);  break;
            default: break;
            }
        }

        // actualizar logica
        switch (current) {
        case GameScreen::SPLASH:       next = splash.update(dt); break;
        case GameScreen::MAIN_MENU:    menu.update(dt);           break;
        case GameScreen::USER_AUTH:    auth.update(dt);           break;
        case GameScreen::LEVEL_SELECT: levelSel.update(dt);       break;
        case GameScreen::GAME:         game.update(dt);           break;
        case GameScreen::CREDITS:      credits.update(dt);        break;
        default: break;
        }

        // transicion de pantalla
        if (next != current) {
            current = next;
            switch (current) {
            case GameScreen::MAIN_MENU:
                // restaurar tamano original al volver al menu
                window.setSize({VENTANA_W, VENTANA_H});
                menu.onEnter();
                switchMusic(music, MUSIC_SCREENS, currentMusicFile);
                currentMusicFile = MUSIC_SCREENS;
                break;
            case GameScreen::USER_AUTH:
                // restaurar tamano por si viene del juego
                window.setSize({VENTANA_W, VENTANA_H});
                auth.onEnter();
                switchMusic(music, MUSIC_SCREENS, currentMusicFile);
                currentMusicFile = MUSIC_SCREENS;
                break;
            case GameScreen::LEVEL_SELECT:
                // restaurar tamano por si viene del juego
                window.setSize({VENTANA_W, VENTANA_H});
                levelSel.onEnter();
                switchMusic(music, MUSIC_SCREENS, currentMusicFile);
                currentMusicFile = MUSIC_SCREENS;
                break;
            case GameScreen::GAME:
                game.onEnter(); // el juego ajusta su propio tamano
                switchMusic(music, MUSIC_GAME, currentMusicFile);
                currentMusicFile = MUSIC_GAME;
                break;
            case GameScreen::SCORES:
                window.setSize({VENTANA_W, VENTANA_H});
                switchMusic(music, MUSIC_SCREENS, currentMusicFile);
                currentMusicFile = MUSIC_SCREENS;
                break;
            case GameScreen::CREDITS:
                window.setSize({VENTANA_W, VENTANA_H});
                credits.onEnter();
                switchMusic(music, MUSIC_SCREENS, currentMusicFile);
                currentMusicFile = MUSIC_SCREENS;
                break;
            default: break;
            }
        }

        // dibujar pantalla activa
        window.clear();
        switch (current) {
        case GameScreen::SPLASH:       splash.draw();           break;
        case GameScreen::MAIN_MENU:    menu.draw();             break;
        case GameScreen::USER_AUTH:    auth.draw();             break;
        case GameScreen::LEVEL_SELECT: levelSel.draw();         break;
        case GameScreen::GAME:         game.draw();             break;
        case GameScreen::SCORES:       scores.draw(window, ""); break;
        case GameScreen::CREDITS:      credits.draw();          break;
        default: break;
        }
        window.display();
    }

    return 0;
}
