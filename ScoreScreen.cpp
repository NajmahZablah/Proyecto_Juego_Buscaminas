#include "ScoreScreen.h"
#include "ScreenManager.h"
#include <algorithm>
#include <sstream>
#include <fstream>

const std::string ScoreScreen::RUTA_SCORES = "scores.txt";

ScoreScreen::ScoreScreen() {
    m_fuente.loadFromFile("C:/Windows/Fonts/arial.ttf");
    m_textoTitulo.setFont(m_fuente);
    m_textoTitulo.setCharacterSize(36);
    m_textoTitulo.setStyle(sf::Text::Bold);
    m_textoTitulo.setFillColor(sf::Color::White);
    configurarBotones(600.f, 680.f);
    cargarPuntajes();
}

void ScoreScreen::configurarBotones(float W, float H) {
    float cx = W / 2.f;

    // 4 pestanas que caben en 600px
    // bajadas a Y=95 para que no se encimen con el titulo
    float btnW   = 126.f;
    float gap    = 8.f;
    float totalW = 4 * btnW + 3 * gap;
    float startX = cx - totalW / 2.f + btnW / 2.f;
    float btnY   = 95.f;  // bajado respecto a version anterior

    m_btnPrincipiante.setup(m_fuente, "Principiante", {startX,                btnY}, {btnW, 38.f}, 11);
    m_btnIntermedio.setup  (m_fuente, "Intermedio",   {startX + btnW + gap,   btnY}, {btnW, 38.f}, 11);
    m_btnExperto.setup     (m_fuente, "Experto",      {startX + (btnW+gap)*2, btnY}, {btnW, 38.f}, 11);
    m_btnCompetitivo.setup (m_fuente, "Competitivo",  {startX + (btnW+gap)*3, btnY}, {btnW, 38.f}, 11);

    m_btnPrincipiante.normalColor = {22,  88,  48, 200};
    m_btnPrincipiante.hoverColor  = {40, 150,  80, 230};
    m_btnIntermedio.normalColor   = {88,  68,  14, 200};
    m_btnIntermedio.hoverColor    = {160,120,  20, 230};
    m_btnExperto.normalColor      = {100, 16,  16, 200};
    m_btnExperto.hoverColor       = {180, 30,  30, 230};
    m_btnCompetitivo.normalColor  = { 20, 50, 100, 200};
    m_btnCompetitivo.hoverColor   = { 40,100, 190, 230};

    m_btnVolver.setup(m_fuente, "< Volver", {cx, H - 50.f}, {200.f, 42.f}, 16);
    m_btnVolver.normalColor = {20, 20,  60, 220};
    m_btnVolver.hoverColor  = {40, 40, 120, 240};
}

void ScoreScreen::cargarPuntajes() {
    m_puntajes.clear();
    m_puntajes["Principiante"] = {};
    m_puntajes["Intermedio"]   = {};
    m_puntajes["Experto"]      = {};
    m_puntajes["Competitivo"]  = {};

    std::ifstream archivo(RUTA_SCORES);
    if (!archivo) return;

    std::string linea;
    while (std::getline(archivo, linea)) {
        if (linea.empty()) continue;
        std::istringstream f(linea);
        std::string nivel, nombre, tStr;
        if (std::getline(f, nivel, ',') &&
            std::getline(f, nombre, ',') &&
            std::getline(f, tStr, ',')) {
            EntradaPuntaje e;
            e.nombreJugador = nombre;
            e.tiempo = std::stof(tStr);
            m_puntajes[nivel].push_back(e);
        }
    }
    for (auto& par : m_puntajes)
        std::sort(par.second.begin(), par.second.end(),
                  [](const EntradaPuntaje& a, const EntradaPuntaje& b) {
                      return a.tiempo < b.tiempo;
                  });
}

void ScoreScreen::agregarPuntaje(const std::string& nombre, float tiempo,
                                 const std::string& nivel) {
    if (tiempo <= 0.f) return;
    m_puntajes[nivel].push_back({nombre, tiempo});
    std::sort(m_puntajes[nivel].begin(), m_puntajes[nivel].end(),
              [](const EntradaPuntaje& a, const EntradaPuntaje& b) {
                  return a.tiempo < b.tiempo;
              });
    if (m_puntajes[nivel].size() > 10) m_puntajes[nivel].resize(10);

    std::ofstream archivo(RUTA_SCORES, std::ios::trunc);
    for (auto& par : m_puntajes)
        for (auto& e : par.second)
            archivo << par.first << "," << e.nombreJugador << "," << e.tiempo << "\n";
}

void ScoreScreen::addScore(const std::string& nombre, float tiempo,
                           const std::string& nivel) {
    agregarPuntaje(nombre, tiempo, nivel);
}

void ScoreScreen::draw(sf::RenderWindow& ventana, const std::string&) {
    float W  = static_cast<float>(ventana.getSize().x);
    float H  = static_cast<float>(ventana.getSize().y);
    float cx = W / 2.f;

    float COL_NUM    = cx - W * 0.40f;
    float COL_NOMBRE = cx - W * 0.30f;
    float COL_TIEMPO = cx + W * 0.20f;

    ventana.draw(UI::makeGradientBackground(
        static_cast<unsigned>(W), static_cast<unsigned>(H),
        {4, 7, 22}, {8, 22, 62}));

    // titulo
    m_textoTitulo.setString("Ranking - " + m_pestanaActiva);
    centrarTexto(m_textoTitulo, cx, 30.f);
    ventana.draw(m_textoTitulo);

    // separador bajo titulo
    sf::RectangleShape sep({W * 0.75f, 1.f});
    sep.setOrigin(sep.getSize().x / 2.f, 0.f);
    sep.setPosition(cx, 56.f);
    sep.setFillColor({55, 100, 200, 100});
    ventana.draw(sep);

    // etiqueta modo
    bool esComp = (m_pestanaActiva == "Competitivo");
    sf::Text etModo; etModo.setFont(m_fuente);
    etModo.setString(esComp ? "[ Modo Competitivo ]" : "[ Modo Libre ]");
    etModo.setCharacterSize(12);
    etModo.setFillColor(esComp ? sf::Color{255,165,30} : sf::Color{65,180,255});
    centrarTexto(etModo, cx, 70.f);
    ventana.draw(etModo);

    // pestanas
    dibujarPestana(ventana, m_btnPrincipiante, m_pestanaActiva == "Principiante", {40, 200,  80});
    dibujarPestana(ventana, m_btnIntermedio,   m_pestanaActiva == "Intermedio",   {200,180,  40});
    dibujarPestana(ventana, m_btnExperto,      m_pestanaActiva == "Experto",       {200, 60,  60});
    dibujarPestana(ventana, m_btnCompetitivo,  m_pestanaActiva == "Competitivo",  { 40,140, 255});

    // divisor visual libre | competitivo
    float divX = m_btnExperto.shape.getPosition().x + 67.f;
    sf::RectangleShape div({2.f, 40.f});
    div.setPosition(divX, 75.f);
    div.setFillColor({90, 90, 120, 130});
    ventana.draw(div);

    // separador bajo pestanas
    sf::RectangleShape sepP({W * 0.9f, 1.f});
    sepP.setOrigin(sepP.getSize().x / 2.f, 0.f);
    sepP.setPosition(cx, 140.f);
    sepP.setFillColor({40, 70, 130, 80});
    ventana.draw(sepP);

    // encabezado columnas
    sf::Text encJ; encJ.setFont(m_fuente); encJ.setCharacterSize(13);
    encJ.setFillColor({80,130,200}); encJ.setStyle(sf::Text::Bold);
    encJ.setString("#  Jugador"); encJ.setPosition(COL_NUM, 146.f);
    ventana.draw(encJ);

    sf::Text encT; encT.setFont(m_fuente); encT.setCharacterSize(13);
    encT.setFillColor({80,130,200}); encT.setStyle(sf::Text::Bold);
    encT.setString(esComp ? "Tiempo (3 niveles)" : "Tiempo");
    encT.setPosition(COL_TIEMPO, 146.f);
    ventana.draw(encT);

    // lista de puntajes
    auto& lista = m_puntajes[m_pestanaActiva];
    if (lista.empty()) {
        sf::Text sd; sd.setFont(m_fuente); sd.setCharacterSize(16);
        sd.setFillColor({80,100,140});
        sd.setString("Aun no hay puntajes registrados.");
        centrarTexto(sd, cx, H / 2.f); ventana.draw(sd);
    } else {
        for (int i = 0; i < (int)lista.size() && i < 10; i++) {
            float posY = 166.f + i * 36.f;
            sf::Color col;
            if      (i == 0) col = {255,215, 50};
            else if (i == 1) col = {200,200,210};
            else if (i == 2) col = {200,130, 60};
            else             col = {180,195,215};

            if (i % 2 == 0) {
                sf::RectangleShape fila({W * 0.88f, 32.f});
                fila.setOrigin(fila.getSize().x / 2.f, 0.f);
                fila.setPosition(cx, posY);
                fila.setFillColor({255,255,255, 8});
                ventana.draw(fila);
            }

            sf::Text tN; tN.setFont(m_fuente); tN.setCharacterSize(18);
            tN.setFillColor(col);
            tN.setString(std::to_string(i+1) + ".");
            tN.setPosition(COL_NUM, posY); ventana.draw(tN);

            sf::Text tNom; tNom.setFont(m_fuente); tNom.setCharacterSize(18);
            tNom.setFillColor(col);
            tNom.setString(lista[i].nombreJugador);
            tNom.setPosition(COL_NOMBRE, posY); ventana.draw(tNom);

            int seg = static_cast<int>(lista[i].tiempo);
            std::ostringstream oss;
            if (seg/60 > 0) oss << seg/60 << "m " << seg%60 << "s";
            else             oss << seg << "s";

            sf::Text tT; tT.setFont(m_fuente); tT.setCharacterSize(18);
            tT.setFillColor(col);
            tT.setString(oss.str());
            tT.setPosition(COL_TIEMPO, posY); ventana.draw(tT);
        }
    }

    if (esComp) {
        sf::Text nota; nota.setFont(m_fuente); nota.setCharacterSize(12);
        nota.setFillColor({80,120,170});
        nota.setString("Tiempo = suma de los 3 niveles completados en orden.");
        centrarTexto(nota, cx, H - 80.f); ventana.draw(nota);
    }

    m_btnVolver.update(ventana);
    m_btnVolver.draw(ventana);
}

GameScreen ScoreScreen::handleEvent(const sf::Event& evento) {
    if (m_btnPrincipiante.isClicked(evento)) m_pestanaActiva = "Principiante";
    if (m_btnIntermedio.isClicked(evento))   m_pestanaActiva = "Intermedio";
    if (m_btnExperto.isClicked(evento))       m_pestanaActiva = "Experto";
    if (m_btnCompetitivo.isClicked(evento))   m_pestanaActiva = "Competitivo";
    if (m_btnVolver.isClicked(evento))        return GameScreen::MAIN_MENU;
    return GameScreen::SCORES;
}

void ScoreScreen::dibujarPestana(sf::RenderWindow& ventana, Button& boton,
                                 bool activa, sf::Color colorLinea) {
    if (activa) {
        sf::RectangleShape sub({126.f, 3.f});
        sf::Vector2f pos = boton.shape.getPosition();
        sub.setPosition(pos.x - 63.f, pos.y + 19.f);
        sub.setFillColor(colorLinea);
        ventana.draw(sub);
        boton.shape.setOutlineColor({colorLinea.r, colorLinea.g, colorLinea.b, 190});
    } else {
        boton.shape.setOutlineColor({42, 60, 100, 80});
    }
    boton.update(ventana);
    boton.draw(ventana);
}

void ScoreScreen::centrarTexto(sf::Text& t, float x, float y) {
    sf::FloatRect r = t.getLocalBounds();
    t.setOrigin(r.left + r.width / 2.f, r.top + r.height / 2.f);
    t.setPosition(x, y);
}
