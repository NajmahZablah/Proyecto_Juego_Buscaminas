#include "ScoreScreen.h"
#include "ScreenManager.h"
#include <algorithm>
#include <sstream>

ScoreScreen::ScoreScreen() {
    m_fuente.loadFromFile("C:/Windows/Fonts/arial.ttf");

    // Título
    m_textoTitulo.setFont(m_fuente);
    m_textoTitulo.setCharacterSize(38);
    m_textoTitulo.setStyle(sf::Text::Bold);
    m_textoTitulo.setFillColor(sf::Color::White);

    configurarBotones(1200.f, 700.f);
    cargarPuntajes();
}

void ScoreScreen::configurarBotones(float anchoVentana, float altoVentana) {
    const float centroX = anchoVentana / 2.f;
    const float separacion = 175.f;

    m_btnPrincipiante.setup(m_fuente, "Principiante", {centroX - separacion * 1.5f, 80.f}, {160.f, 42.f }, 14);
    m_btnPrincipiante.normalColor = {22, 88, 48, 200};
    m_btnPrincipiante.hoverColor = {40, 150, 80, 230};

    m_btnIntermedio.setup(m_fuente, "Intermedio", {centroX - separacion * 0.5f, 80.f }, {160.f, 42.f }, 14);
    m_btnIntermedio.normalColor = {88, 68, 14, 200};
    m_btnIntermedio.hoverColor = {160, 120, 20, 230};

    m_btnExperto.setup(m_fuente, "Experto", {centroX + separacion * 0.5f, 80.f}, {160.f, 42.f }, 14);
    m_btnExperto.normalColor = {100, 16, 16, 200};
    m_btnExperto.hoverColor = {180, 30, 30, 230};

    m_btnCompetitivo.setup(m_fuente, "Competitivo", {centroX + separacion * 1.5f, 80.f}, {160.f, 42.f }, 14);
    m_btnCompetitivo.normalColor = {20,  50, 100, 200};
    m_btnCompetitivo.hoverColor = {40, 100, 190, 230};

    // Botón volver
    m_btnVolver.setup(m_fuente, "< Volver", {centroX, altoVentana - 55.f}, {230.f, 50.f }, 22);
    m_btnVolver.normalColor = {20, 20,  60, 220};
    m_btnVolver.hoverColor = {40, 40, 120, 240};
}

void ScoreScreen::cargarPuntajes() {
    m_puntajes["Principiante"] = {};
    m_puntajes["Intermedio"] = {};
    m_puntajes["Experto"] = {};
    m_puntajes["Competitivo"] = {};

    // Puntajes de demostración
    // Contraseña de cada usuario: nombre en minúsculas + "123"
    // Ejemplo: usuario "Valeria" -> contrasena "valeria123"

    // Principiante
    agregarPuntaje("Valeria",   18.f, "Principiante");
    agregarPuntaje("Marcos",    22.f, "Principiante");
    agregarPuntaje("Lucia",     25.f, "Principiante");
    agregarPuntaje("Diego",     28.f, "Principiante");
    agregarPuntaje("Sofia",     31.f, "Principiante");
    agregarPuntaje("Andres",    35.f, "Principiante");
    agregarPuntaje("Camila",    38.f, "Principiante");
    agregarPuntaje("Roberto",   42.f, "Principiante");
    agregarPuntaje("Isabella",  47.f, "Principiante");
    agregarPuntaje("Fernando",  53.f, "Principiante");

    // Intermedio
    agregarPuntaje("Marcos",    142.f, "Intermedio");
    agregarPuntaje("Diego",     158.f, "Intermedio");
    agregarPuntaje("Valeria",   165.f, "Intermedio");
    agregarPuntaje("Roberto",   174.f, "Intermedio");
    agregarPuntaje("Andres",    183.f, "Intermedio");
    agregarPuntaje("Sofia",     191.f, "Intermedio");
    agregarPuntaje("Fernando",  205.f, "Intermedio");
    agregarPuntaje("Camila",    218.f, "Intermedio");
    agregarPuntaje("Lucia",     234.f, "Intermedio");
    agregarPuntaje("Isabella",  249.f, "Intermedio");

    // Experto
    agregarPuntaje("Diego",     312.f, "Experto");
    agregarPuntaje("Marcos",    335.f, "Experto");
    agregarPuntaje("Roberto",   358.f, "Experto");
    agregarPuntaje("Andres",    374.f, "Experto");
    agregarPuntaje("Valeria",   391.f, "Experto");
    agregarPuntaje("Fernando",  415.f, "Experto");
    agregarPuntaje("Sofia",     438.f, "Experto");
    agregarPuntaje("Isabella",  462.f, "Experto");
    agregarPuntaje("Camila",    487.f, "Experto");
    agregarPuntaje("Lucia",     513.f, "Experto");

    // Competitivo (suma Principiante + Intermedio + Experto)
    agregarPuntaje("Diego",     498.f, "Competitivo");
    agregarPuntaje("Marcos",    499.f, "Competitivo");
    agregarPuntaje("Valeria",   574.f, "Competitivo");
    agregarPuntaje("Roberto",   574.f, "Competitivo");
    agregarPuntaje("Andres",    592.f, "Competitivo");
    agregarPuntaje("Sofia",     660.f, "Competitivo");
    agregarPuntaje("Fernando",  673.f, "Competitivo");
    agregarPuntaje("Camila",    743.f, "Competitivo");
    agregarPuntaje("Isabella",  758.f, "Competitivo");
    agregarPuntaje("Lucia",     772.f, "Competitivo");
}

void ScoreScreen::agregarPuntaje(const std::string& nombreJugador, float tiempo, const std::string& nivel) {
    if (tiempo <= 0.f) {
        return;
    }

    m_puntajes[nivel].push_back({ nombreJugador, tiempo });

    std::sort(m_puntajes[nivel].begin(), m_puntajes[nivel].end(), [](const EntradaPuntaje& a, const EntradaPuntaje& b) {
        return a.tiempo < b.tiempo;
    });

    if (m_puntajes[nivel].size() > 10) {
        m_puntajes[nivel].resize(10);
    }
}

void ScoreScreen::addScore(const std::string& nombreJugador, float tiempo, const std::string& nivel) {
    agregarPuntaje(nombreJugador, tiempo, nivel);
}

void ScoreScreen::draw(sf::RenderWindow& ventana, const std::string& nivel) {
    const float anchoVentana = static_cast<float>(ventana.getSize().x);
    const float altoVentana = static_cast<float>(ventana.getSize().y);
    const float centroX = anchoVentana / 2.f;

    // Coordenadas X fijas para las dos columnas — esto garantiza alineación
    const float COL_NUMERO = centroX - 280.f; // "#  1."
    const float COL_NOMBRE = centroX - 230.f; // "Valeria"
    const float COL_TIEMPO = centroX + 100.f; // "18s"

    // Fondo
    ventana.draw(UI::makeGradientBackground(static_cast<unsigned>(anchoVentana), static_cast<unsigned>(altoVentana),
                                            {4, 7, 22}, {8, 22, 62}));

    // Título
    m_textoTitulo.setString("Ranking - " + m_pestanaActiva);
    centrarTexto(m_textoTitulo, centroX, 28.f);
    ventana.draw(m_textoTitulo);

    // Línea decorativa bajo el título
    sf::RectangleShape separadorTitulo({420.f, 1.f});
    separadorTitulo.setOrigin(210.f, 0.f);
    separadorTitulo.setPosition(centroX, 54.f);
    separadorTitulo.setFillColor({55, 100, 200, 100});
    ventana.draw(separadorTitulo);

    // Pestañas
    dibujarPestana(ventana, m_btnPrincipiante,m_pestanaActiva == "Principiante", {40, 200, 80});
    dibujarPestana(ventana, m_btnIntermedio, m_pestanaActiva == "Intermedio", {200, 180, 40});
    dibujarPestana(ventana, m_btnExperto, m_pestanaActiva == "Experto", {200, 60, 60});
    dibujarPestana(ventana, m_btnCompetitivo, m_pestanaActiva == "Competitivo", {40, 140, 255});

    // Línea separadora bajo las pestañas
    sf::RectangleShape separadorPestanas({ anchoVentana * 0.7f, 1.f });
    separadorPestanas.setOrigin(separadorPestanas.getSize().x / 2.f, 0.f);
    separadorPestanas.setPosition(centroX, 108.f);
    separadorPestanas.setFillColor({40, 70, 130, 80});
    ventana.draw(separadorPestanas);

    // Encabezado de columnas con posición X fija
    bool esCompetitivo = (m_pestanaActiva == "Competitivo");

    sf::Text encJugador;
    encJugador.setFont(m_fuente);
    encJugador.setCharacterSize(14);
    encJugador.setFillColor({80, 130, 200});
    encJugador.setStyle(sf::Text::Bold);
    encJugador.setString("#     Jugador");
    encJugador.setPosition(COL_NUMERO, 118.f);
    ventana.draw(encJugador);

    sf::Text encTiempo;
    encTiempo.setFont(m_fuente);
    encTiempo.setCharacterSize(14);
    encTiempo.setFillColor({80, 130, 200});
    encTiempo.setStyle(sf::Text::Bold);
    encTiempo.setString(esCompetitivo ? "Tiempo total (suma 3 niveles)" : "Tiempo");
    encTiempo.setPosition(COL_TIEMPO, 118.f);
    ventana.draw(encTiempo);

    // Lista de puntajes
    auto& lista = m_puntajes[m_pestanaActiva];

    if (lista.empty()) {
        sf::Text textoVacio;
        textoVacio.setFont(m_fuente);
        textoVacio.setCharacterSize(18);
        textoVacio.setFillColor({80, 100, 140});
        textoVacio.setString("Aun no hay puntajes registrados.");
        centrarTexto(textoVacio, centroX, 280.f);
        ventana.draw(textoVacio);

    } else {
        for (int i = 0; i < static_cast<int>(lista.size()) && i < 10; i++) {
            const EntradaPuntaje& entrada = lista[i];
            const float posY = 142.f + i * 38.f;

            // Dorado, plateado, bronce para el top 3
            sf::Color colorEntrada;
            if (i == 0) {
                colorEntrada = { 255, 215,  50 };
            } else if (i == 1) {
                colorEntrada = { 200, 200, 210 };
            } else if (i == 2) {
                colorEntrada = { 200, 130,  60 };
            } else {
                colorEntrada = { 180, 195, 215 };
            }

            // Fondo alterno en filas pares
            if (i % 2 == 0) {
                sf::RectangleShape fondoFila({ anchoVentana * 0.62f, 34.f });
                fondoFila.setOrigin(fondoFila.getSize().x / 2.f, 0.f);
                fondoFila.setPosition(centroX, posY);
                fondoFila.setFillColor({255, 255, 255, 8});
                ventana.draw(fondoFila);
            }

            // Columna número
            sf::Text textoNumero;
            textoNumero.setFont(m_fuente);
            textoNumero.setCharacterSize(20);
            textoNumero.setFillColor(colorEntrada);
            textoNumero.setString(std::to_string(i + 1) + ".");
            textoNumero.setPosition(COL_NUMERO, posY);
            ventana.draw(textoNumero);

            // Columna nombre
            sf::Text textoNombre;
            textoNombre.setFont(m_fuente);
            textoNombre.setCharacterSize(20);
            textoNombre.setFillColor(colorEntrada);
            textoNombre.setString(entrada.nombreJugador);
            textoNombre.setPosition(COL_NOMBRE, posY);
            ventana.draw(textoNombre);

            // Columna tiempo — X fija garantiza alineación perfecta
            int segundosTotales = static_cast<int>(entrada.tiempo);
            int minutos = segundosTotales / 60;
            int segundos = segundosTotales % 60;

            std::ostringstream flujoTiempo;
            if (minutos > 0) {
                flujoTiempo << minutos << "m " << segundos << "s";
            } else {
                flujoTiempo << segundos << "s";
            }

            sf::Text textoTiempo;
            textoTiempo.setFont(m_fuente);
            textoTiempo.setCharacterSize(20);
            textoTiempo.setFillColor(colorEntrada);
            textoTiempo.setString(flujoTiempo.str());
            textoTiempo.setPosition(COL_TIEMPO, posY);
            ventana.draw(textoTiempo);
        }
    }

    // Nota en pestaña competitivo
    if (esCompetitivo) {
        sf::Text nota;
        nota.setFont(m_fuente);
        nota.setCharacterSize(15);
        nota.setFillColor({80, 120, 170});
        nota.setString("El tiempo es la suma de los 3 niveles completados en orden: "
                       "Principiante + Intermedio + Experto.");
        centrarTexto(nota, centroX, altoVentana - 100.f);
        ventana.draw(nota);
    }

    // Botón volver
    m_btnVolver.update(ventana);
    m_btnVolver.draw(ventana);
}

GameScreen ScoreScreen::handleEvent(const sf::Event& evento) {
    if (m_btnPrincipiante.isClicked(evento)) {
        m_pestanaActiva = "Principiante";
    }
    if (m_btnIntermedio.isClicked(evento)) {
        m_pestanaActiva = "Intermedio";
    }
    if (m_btnExperto.isClicked(evento)) {
        m_pestanaActiva = "Experto";
    }
    if (m_btnCompetitivo.isClicked(evento)) {
        m_pestanaActiva = "Competitivo";
    }
    if (m_btnVolver.isClicked(evento)) {
        return GameScreen::MAIN_MENU;
    }

    return GameScreen::SCORES;
}

void ScoreScreen::dibujarPestana(sf::RenderWindow& ventana, Button& boton, bool activa, sf::Color colorLinea) {
    if (activa) {
        sf::RectangleShape subrayado({160.f, 3.f});
        sf::Vector2f posBoton = boton.shape.getPosition();
        subrayado.setPosition(posBoton.x - 80.f, posBoton.y + 21.f);
        subrayado.setFillColor(colorLinea);
        ventana.draw(subrayado);
        boton.shape.setOutlineColor({colorLinea.r, colorLinea.g, colorLinea.b, 190});
    } else {
        boton.shape.setOutlineColor({42, 60, 100, 80});
    }

    boton.update(ventana);
    boton.draw(ventana);
}

void ScoreScreen::centrarTexto(sf::Text& texto, float x, float y) {
    sf::FloatRect limites = texto.getLocalBounds();
    texto.setOrigin(limites.left + limites.width  / 2.f, limites.top  + limites.height / 2.f);
    texto.setPosition(x, y);
}
