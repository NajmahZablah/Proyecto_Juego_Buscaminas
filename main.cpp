/*  main.cpp — Buscaminas
 *  Punto de entrada del programa
*/

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cstdlib>
#include <ctime>
#include <string>
#include "ScreenManager.h"
#include "SplashScreen.h"
#include "MainMenuScreen.h"
#include "UserAuthScreen.h"
#include "LevelSelectScreen.h"
#include "GameScreen.h"
#include "ScoreScreen.h"

// Cambia la música solo si es diferente a la que ya suena
void cambiarMusica(sf::Music& musica, const std::string& archivoNuevo, std::string& archivoActual) {
    if (archivoActual == archivoNuevo) {
        return;
    }

    musica.stop();

    if (musica.openFromFile(archivoNuevo)) {
        musica.setLoop(true);
        musica.setVolume(45.f);
        musica.play();
        archivoActual = archivoNuevo;
    }
}

int main() {

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Ventana principal
    sf::RenderWindow ventana(sf::VideoMode(1200, 700), "Buscaminas", sf::Style::Close | sf::Style::Titlebar);
    ventana.setFramerateLimit(60);

    // Fuente
    sf::Font fuente;
    if (!fuente.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        fuente.loadFromFile("C:/Windows/Fonts/calibri.ttf");
    }

    // Música
    const std::string MUSICA_MENUS = "assets/MusicaScreens.wav";
    const std::string MUSICA_JUEGO = "assets/MusicaGame.wav";
    sf::Music musica;
    std::string archivoMusicaActual = "";
    cambiarMusica(musica, MUSICA_MENUS, archivoMusicaActual);

    // Estado global
    PlayerData jugador;
    LevelConfig nivelElegido = NIVELES[0];

    // Pantallas
    SplashScreen splash (ventana, fuente);
    MainMenuScreen menu (ventana, fuente, jugador);
    UserAuthScreen auth (ventana, fuente, jugador);
    LevelSelectScreen selectorNivel(ventana, fuente, jugador, nivelElegido);
    ScoreScreen puntajes;
    PantallaJuego juego (ventana, fuente, jugador, nivelElegido, puntajes);

    puntajes.cargarPuntajes();
    splash.onEnter();

    // Estado de pantalla
    GameScreen pantallaActual = GameScreen::SPLASH;
    GameScreen pantallaAnterior = GameScreen::SPLASH;

    sf::Clock reloj;

    // Game loop
    while (ventana.isOpen()) {

        float dt = reloj.restart().asSeconds();
        if (dt > 0.05f) {
            dt = 0.05f;
        }

        // Procesar eventos
        sf::Event evento;
        while (ventana.pollEvent(evento)) {
            if (evento.type == sf::Event::Closed) {
                ventana.close();
            }

            switch (pantallaActual) {
                case GameScreen::SPLASH: {
                    splash.handleEvent(evento);
                    break;
                }
                case GameScreen::MAIN_MENU: {
                    // PUNTAJES va directo a ScoreScreen, sin pasar por LevelSelect
                    pantallaActual = menu.handleEvent(evento);
                    break;
                }
                case GameScreen::USER_AUTH: {
                    pantallaActual = auth.handleEvent(evento);
                    break;
                }
                case GameScreen::LEVEL_SELECT: {
                    pantallaActual = selectorNivel.handleEvent(evento);
                    break;
                }
                case GameScreen::GAME: {
                    pantallaActual = juego.handleEvent(evento);
                    break;
                }
                case GameScreen::SCORES: {
                    pantallaActual = puntajes.handleEvent(evento);
                    break;
                }
                default: {
                    break;
                }
            }
        }

        // Actualizar lógica
        switch (pantallaActual) {
            case GameScreen::SPLASH: {
                pantallaActual = splash.update(dt);
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
                selectorNivel.update(dt);
                break;
            }
            case GameScreen::GAME: {
                juego.update(dt);
                break;
            }
            default: {
                break;
            }
        }

        // Transiciones: onEnter y música
        if (pantallaActual != pantallaAnterior) {
            switch (pantallaActual) {
                case GameScreen::MAIN_MENU: {
                    menu.onEnter();
                    cambiarMusica(musica, MUSICA_MENUS, archivoMusicaActual);
                    break;
                }
                case GameScreen::USER_AUTH: {
                    auth.onEnter();
                    cambiarMusica(musica, MUSICA_MENUS, archivoMusicaActual);
                    break;
                }
                case GameScreen::LEVEL_SELECT: {
                    selectorNivel.onEnter();
                    // En competitivo, preseleccionar el nivel que corresponde
                    if (jugador.modoJuego == ModoJuego::COMPETITIVO) {
                        nivelElegido = NIVELES[jugador.nivelCompetitivoActual];
                    }
                    cambiarMusica(musica, MUSICA_MENUS, archivoMusicaActual);
                    break;
                }
                case GameScreen::GAME: {
                    juego.onEnter();
                    cambiarMusica(musica, MUSICA_JUEGO, archivoMusicaActual);
                    break;
                }
                case GameScreen::SCORES: {
                    cambiarMusica(musica, MUSICA_MENUS, archivoMusicaActual);
                    break;
                }
                default: {
                    break;
                }
            }

            pantallaAnterior = pantallaActual;
        }

        // Dibujar
        ventana.clear();

        switch (pantallaActual) {
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
                selectorNivel.draw();
                break;
            }
            case GameScreen::GAME: {
                juego.draw();
                break;
            }
            case GameScreen::SCORES: {
                puntajes.draw(ventana, nivelElegido.nombre);
                break;
            }
            default: {
                break;
            }
        }

        ventana.display();
    }

    return 0;
}
