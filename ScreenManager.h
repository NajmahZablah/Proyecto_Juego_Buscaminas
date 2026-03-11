#pragma once
#include <SFML/Graphics.hpp>
#include <string>

/*ScreenManager.
 * Define los estados posibles del juego y los datos
 * del jugador que se comparten entre todas las pantallas.
*/

// Estados de pantalla
// main.cpp usa este enum para saber qué pantalla dibujar.
enum class GameScreen {
    SPLASH,        // presentación animada al iniciar
    MAIN_MENU,     // menú principal
    USER_AUTH,     // login / registro de usuario
    LEVEL_SELECT,  // selección de nivel
    GAME           // tablero de Buscaminas
};

//  Datos del jugador activo
// Se pasa por referencia entre pantallas para que todas
struct PlayerData {
    std::string username  = "";    // nombre de usuario
    std::string password  = "";    // contraseña
    int bestScoreEasy     = 0;     // mejor tiempo nivel Fácil (seg)
    int bestScoreMed      = 0;     // mejor tiempo nivel Intermedio
    int bestScoreHard     = 0;     // mejor tiempo nivel Experto
    bool loggedIn         = false; // true si hay sesión activa
};
