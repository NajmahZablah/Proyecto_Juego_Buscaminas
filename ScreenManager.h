#pragma once
#include <SFML/Graphics.hpp>
#include <string>

//  ScreenManager.h
//  Define los estados posibles del juego y los datos
//  del jugador que se comparten entre todas las pantallas.

// todos los estados posibles de pantalla
// main.cpp usa este enum para saber que pantalla dibujar
enum class GameScreen {
    SPLASH,        // presentacion animada al iniciar
    MAIN_MENU,     // menu principal
    USER_AUTH,     // login / registro de usuario
    LEVEL_SELECT,  // seleccion de nivel
    GAME,          // tablero de buscaminas
    SCORES,        // pantalla de ranking
    CREDITS        // pantalla de creditos
};

// modo de juego del jugador
enum class ModoJuego {
    LIBRE,
    COMPETITIVO
};

//  datos del jugador activo
//  se pasa por referencia entre pantallas para que todas
//  vean los mismos datos sin duplicarlos
struct PlayerData {
    std::string nombre        = "";
    std::string contrasena    = "";

    int mejorTiempoFacil        = 0;  // mejor tiempo nivel facil (seg)
    int mejorTiempoMedio        = 0;  // mejor tiempo nivel intermedio
    int mejorTiempoDificil      = 0;  // mejor tiempo nivel experto
    int mejorTiempoCompetitivo  = 0;  // mejor tiempo modo competitivo

    bool sesionActiva = false;        // true si hay sesion activa

    // datos para el modo competitivo
    ModoJuego modoJuego              = ModoJuego::LIBRE;
    int       nivelCompetitivoActual = 0;   // 0,1,2 -> en que nivel va
    float     tiempoAcumulado        = 0.f; // suma de tiempos en competitivo
};
