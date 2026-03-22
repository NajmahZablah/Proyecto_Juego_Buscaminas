#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <map>
#include "UIHelpers.h"

/*  ScoreScreen.h
 *  Pantalla de ranking
 *  LIBRE: Principiante, Intermedio, Experto
 *  COMPETITIVO: suma de los 3 niveles
 *  datos en scores.txt
*/

enum class GameScreen;

struct EntradaPuntaje {
    std::string nombreJugador;
    float tiempo;
};

class ScoreScreen {
public:
    ScoreScreen();
    void agregarPuntaje(const std::string& nombreJugador, float tiempo, const std::string& nivel);
    void cargarPuntajes();
    void draw(sf::RenderWindow& ventana, const std::string& nivel);
    GameScreen handleEvent(const sf::Event& evento);
    void configurarBotones(float anchoVentana, float altoVentana);
    void addScore(const std::string& nombreJugador, float tiempo, const std::string& nivel);

private:
    void dibujarPestana(sf::RenderWindow& ventana, Button& boton, bool activa, sf::Color colorLinea);
    void centrarTexto(sf::Text& texto, float x, float y);

    std::map<std::string, std::vector<EntradaPuntaje>> m_puntajes;
    sf::Font m_fuente;
    sf::Text m_textoTitulo;

    Button m_btnPrincipiante;
    Button m_btnIntermedio;
    Button m_btnExperto;
    Button m_btnCompetitivo;
    Button m_btnVolver;

    std::string m_pestanaActiva = "Principiante";
    static const std::string RUTA_SCORES;
};
