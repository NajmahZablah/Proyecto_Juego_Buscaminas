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
#include <SFML/Audio.hpp>
#include <cstdlib>
#include <ctime>
#include <string>
#include "ScreenManager.h"
#include "UserManager.h"
#include "SplashScreen.h"
#include "MainMenuScreen.h"
#include "UserAuthScreen.h"
#include "LevelSelectScreen.h"
#include "GameScreen.h"
#include "ScoreScreen.h"

void switchMusic(sf::Music& music, const std::string& newFile, std::string& currentFile) {
    if (currentFile == newFile) return;
    music.stop();
    if (music.openFromFile(newFile)) {
        music.setLoop(true);
        music.setVolume(45.f);
        music.play();
        currentFile = newFile;
    }
}

int main() {

    bool viewingScores = false;

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Ventana principal
    sf::RenderWindow window(sf::VideoMode(1200, 700), "Buscaminas", sf::Style::Close | sf::Style::Titlebar);
    window.setFramerateLimit(60);

    // Fuente
    sf::Font font;
    if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        font.loadFromFile("C:/Windows/Fonts/calibri.ttf");
    }

    // Música
    const std::string MUSIC_SCREENS = "assets/MusicaScreens.wav";
    const std::string MUSIC_GAME    = "assets/MusicaGame.wav";
    sf::Music music;
    std::string currentMusicFile = "";
    switchMusic(music, MUSIC_SCREENS, currentMusicFile);

    // Estado global
    PlayerData player;
    LevelConfig chosenLevel = LEVELS[0];
    GameScreen current = GameScreen::SPLASH;

    // Instanciar pantallas
    SplashScreen splash(window, font);
    MainMenuScreen menu(window, font, player);
    UserAuthScreen auth(window, font, player);
    LevelSelectScreen levelSel(window, font, player, chosenLevel);
    ScoreScreen scoreScreen;
    PantallaJuego game(window, font, player, chosenLevel, scoreScreen);

    scoreScreen.loadScores();
    splash.onEnter();

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;

        sf::Event event;

        // ---- PROCESAR EVENTOS ----
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            switch (current) {
            case GameScreen::SPLASH: splash.handleEvent(event); break;
            case GameScreen::MAIN_MENU: {
                GameScreen next = menu.handleEvent(event);

                if (next == GameScreen::SCORES) {
                    viewingScores = true;
                    current = GameScreen::LEVEL_SELECT; // primero elegir nivel
                } else {
                    current = next;
                }
                break;
            }
            case GameScreen::USER_AUTH: current = auth.handleEvent(event); break;
            case GameScreen::LEVEL_SELECT:
                current = levelSel.handleEvent(event);
                break;
            case GameScreen::GAME: current = game.handleEvent(event); break;
            case GameScreen::SCORES: current = scoreScreen.handleEvent(event); break;
            default: break;
            }
        }
        // 🔥 REDIRECCIÓN SEGURA (FUERA DE pollEvent)
        if (current == GameScreen::GAME && viewingScores) {
            current = GameScreen::SCORES;
            viewingScores = false;
        }

        // ---- ACTUALIZAR LOGICA ----
        switch (current) {
        case GameScreen::SPLASH: current = splash.update(dt); break;
        case GameScreen::MAIN_MENU: menu.update(dt); break;
        case GameScreen::USER_AUTH: auth.update(dt); break;
        case GameScreen::LEVEL_SELECT: levelSel.update(dt); break;
        case GameScreen::GAME: game.update(dt); break;
        default: break;
        }

        // ---- TRANSICIONES MUSICALES Y ONENTER ----
        static GameScreen prevScreen = current;
        if (current != prevScreen) {
            switch (current) {
            case GameScreen::MAIN_MENU: menu.onEnter(); switchMusic(music, MUSIC_SCREENS, currentMusicFile); break;
            case GameScreen::USER_AUTH: auth.onEnter(); switchMusic(music, MUSIC_SCREENS, currentMusicFile); break;
            case GameScreen::LEVEL_SELECT: levelSel.onEnter(); switchMusic(music, MUSIC_SCREENS, currentMusicFile); break;
            case GameScreen::GAME: game.onEnter(); switchMusic(music, MUSIC_GAME, currentMusicFile); break;
            default: break;
            }
            prevScreen = current;
        }

        // ---- DIBUJAR ----
        window.clear();
        switch (current) {
        case GameScreen::SPLASH: splash.draw(); break;
        case GameScreen::MAIN_MENU: menu.draw(); break;
        case GameScreen::USER_AUTH: auth.draw(); break;
        case GameScreen::LEVEL_SELECT: levelSel.draw(); break;
        case GameScreen::GAME: game.draw(); break;
        case GameScreen::SCORES: scoreScreen.draw(window, chosenLevel.name); break;
        default: break;
        }
        window.display();


    }

    return 0;
}
