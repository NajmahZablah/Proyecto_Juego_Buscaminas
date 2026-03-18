#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <map>
#include "UIHelpers.h"

/*  ScoreScreen.h
 *  Pantalla de ranking de puntajes
 *
 *  PESTAÑAS:
 *  Principiante - mejores tiempos del nivel fácil
 *  Intermedio- mejores tiempos del nivel medio
 *  Experto - mejores tiempos del nivel difícil
 *  Competitivo - jugadores que completaron los 3 niveles,
 *                ordenados por suma total de tiempo
 *
 *  Cada pestaña muestra hasta 10 entradas ordenadas
 *  de menor a mayor tiempo (menor tiempo = mejor)
*/

// Forward declaration para evitar include circular
enum class GameScreen;

struct EntradaPuntaje {
    std::string nombreJugador;
    float tiempo; // en segundos
};

class ScoreScreen {
public:

    ScoreScreen();

    /* Agrega un puntaje al ranking del nivel indicado
     * El nivel puede ser: Principiante, Intermedio,
     * Experto o Competitivo
    */
    void agregarPuntaje(const std::string& nombreJugador, float tiempo, const std::string& nivel);

    void cargarPuntajes();

    void draw(sf::RenderWindow& ventana, const std::string& nivel);

    GameScreen handleEvent(const sf::Event& evento);

    void configurarBotones(float anchoVentana, float altoVentana);

    // Mantiene compatibilidad con llamadas anteriores desde GameScreen.h
    void addScore(const std::string& nombreJugador, float tiempo, const std::string& nivel);

private:

    void dibujarPestana(sf::RenderWindow& ventana, Button& boton, bool activa, sf::Color colorLinea);

    void centrarTexto(sf::Text& texto, float x, float y);

    // Variables miembro
    std::map<std::string, std::vector<EntradaPuntaje>> m_puntajes;

    sf::Font m_fuente;
    sf::Text m_textoTitulo;
    sf::Text m_textosEntradas[10];

    Button m_btnPrincipiante;
    Button m_btnIntermedio;
    Button m_btnExperto;
    Button m_btnCompetitivo;
    Button m_btnVolver;

    std::string m_pestanaActiva = "Principiante";
};
