#pragma once
#include <SFML/Graphics.hpp>
#include <string>

/*  ScreenManager.h
 *  Define los estados posibles del juego y los datos
 *  del jugador que se comparten entre todas las pantallas
*/

// Estados de pantalla
// main.cpp usa este enum para saber qué pantalla dibujar
enum class GameScreen {
    SPLASH, // presentación animada al iniciar
    MAIN_MENU, // menú principal
    USER_AUTH, // login / registro de usuario
    LEVEL_SELECT, // selección de nivel
    GAME, // tablero de Buscaminas
    SCORES // pantalla de puntajes
};

// Modo de juego elegido en la pantalla de selección de nivel
enum class ModoJuego {
    LIBRE, // el jugador elige cualquier nivel libremente
    COMPETITIVO // debe pasar nivel 1 -> 2 -> 3 en orden
};

/* Datos del jugador activo
 * Se pasa por referencia entre pantallas para que todas
 * lean y escriban el mismo objeto
*/
struct PlayerData {

    // Datos de cuenta
    std::string nombre = ""; // nombre de usuario
    std::string contrasena = ""; // contraseña

    // Mejores tiempos por nivel individual (en segundos, 0 = nunca completado)
    int mejorTiempoFacil = 0;
    int mejorTiempoMedio = 0;
    int mejorTiempoDificil = 0;

    // Mejor tiempo competitivo: suma de los 3 niveles (0 = nunca completado)
    int mejorTiempoCompetitivo = 0;

    // Estado de sesión
    bool sesionActiva = false;

    // Estado de la sesión competitiva en curso
    // Estos campos NO se guardan en disco, se resetean al iniciar cada partida
    ModoJuego modoJuego = ModoJuego::LIBRE;
    int nivelCompetitivoActual = 0; // 0 = no ha empezado, 1 = pasó nivel 1, 2 = pasó nivel 2
    float tiempoAcumulado = 0.f; // suma de tiempos de los niveles ya completados
};
