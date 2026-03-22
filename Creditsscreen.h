#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "ScreenManager.h"
#include "UIHelpers.h"

/*  CreditsScreen.h
 *  Pantalla de creditos del juego
 *  muestra los programadores con nombre, correo y foto (opcional)
 *  si no encuentra la foto muestra un cuadro con la inicial
*/

struct Programador {
    std::string nombre;
    std::string correo;
    std::string rutaFoto;
};

class CreditsScreen {
public:

    CreditsScreen(sf::RenderWindow& ventana, const sf::Font& fuente)
        : m_ventana(ventana), m_fuente(fuente) {}

    void onEnter() {
        const float W  = static_cast<float>(m_ventana.getSize().x);
        const float cx = W / 2.f;

        m_programadores = {
            { "Najmah Zablah",    "najmahzablah@gmail.com",      "assets/foto1.png" },
            { "Alex Santos",      "alexfsantosc@gmail.com",      "assets/foto2.png" },
            { "Nadiesda Fuentes", "nadfuentes.hdez03@gmail.com", "assets/foto3.png" }
        };

        m_fotos.clear();
        m_fotosCargadas.clear();
        for (auto& p : m_programadores) {
            sf::Texture tex;
            bool ok = tex.loadFromFile(p.rutaFoto);
            m_fotos.push_back(tex);
            m_fotosCargadas.push_back(ok);
        }

        m_titulo.setFont(m_fuente);
        m_titulo.setString("CREDITOS");
        m_titulo.setCharacterSize(52);
        m_titulo.setStyle(sf::Text::Bold);
        m_titulo.setFillColor(sf::Color::White);
        centrarTexto(m_titulo, cx, 60.f);

        m_subtitulo.setFont(m_fuente);
        m_subtitulo.setString("Programacion III - Equipo de desarrollo");
        m_subtitulo.setCharacterSize(16);
        m_subtitulo.setFillColor({130, 195, 255});
        centrarTexto(m_subtitulo, cx, 105.f);

        const float H = static_cast<float>(m_ventana.getSize().y);
        m_btnVolver.setup(m_fuente, "< Volver", {cx, H - 50.f}, {200.f, 42.f}, 17);
        m_btnVolver.normalColor = {20, 20, 48, 185};
        m_btnVolver.hoverColor  = {45, 45, 105, 225};

        m_tiempo = 0.f;
    }

    GameScreen handleEvent(const sf::Event& e) {
        if (m_btnVolver.isClicked(e)) return GameScreen::MAIN_MENU;
        if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape)
            return GameScreen::MAIN_MENU;
        return GameScreen::CREDITS;
    }

    void update(float dt) {
        m_btnVolver.update(m_ventana);
        m_tiempo += dt;
    }

    void draw() {
        const float W  = static_cast<float>(m_ventana.getSize().x);
        const float H  = static_cast<float>(m_ventana.getSize().y);
        const float cx = W / 2.f;

        m_ventana.draw(UI::makeGradientBackground(
            static_cast<unsigned>(W), static_cast<unsigned>(H),
            {4, 7, 22}, {8, 22, 62}));

        m_ventana.draw(m_titulo);

        sf::RectangleShape sep({280.f, 1.f});
        sep.setOrigin(140.f, 0.f);
        sep.setPosition(cx, 88.f);
        sep.setFillColor({55, 100, 200, 100});
        m_ventana.draw(sep);

        m_ventana.draw(m_subtitulo);

        // tarjetas centradas entre el subtitulo y el boton volver
        const float CARD_W = 160.f;
        const float CARD_H = 220.f;
        const float GAP    = 28.f;
        int   total  = static_cast<int>(m_programadores.size());
        float totalW = total * CARD_W + (total - 1) * GAP;
        float startX = cx - totalW / 2.f;

        // centrado vertical en el espacio disponible
        float areaTop = 130.f;
        float areaBot = H - 100.f;
        float cardY   = areaTop + (areaBot - areaTop - CARD_H) / 2.f;

        for (int i = 0; i < total; i++) {
            float cardX = startX + i * (CARD_W + GAP);
            dibujarTarjeta(m_programadores[i], cardX, cardY, CARD_W, CARD_H, i);
        }

        m_btnVolver.draw(m_ventana);
    }

private:

    void dibujarTarjeta(const Programador& prog, float x, float y,
                        float w, float h, int idx) {
        sf::RectangleShape card({w, h});
        card.setPosition(x, y);
        card.setFillColor({255, 255, 255, 12});
        card.setOutlineThickness(1.4f);
        card.setOutlineColor({90, 150, 255, 60});
        m_ventana.draw(card);

        float cx     = x + w / 2.f;
        float fotoSz = 100.f;
        float fotoX  = x + (w - fotoSz) / 2.f;
        float fotoY  = y + 14.f;

        if (idx < (int)m_fotosCargadas.size() && m_fotosCargadas[idx]) {
            sf::Sprite sprite(m_fotos[idx]);
            float sx = fotoSz / m_fotos[idx].getSize().x;
            float sy = fotoSz / m_fotos[idx].getSize().y;
            sprite.setScale(std::min(sx, sy), std::min(sx, sy));
            sprite.setPosition(fotoX, fotoY);
            m_ventana.draw(sprite);
        } else {
            sf::RectangleShape cuadro({fotoSz, fotoSz});
            cuadro.setPosition(fotoX, fotoY);
            cuadro.setFillColor({28, 50, 110, 200});
            cuadro.setOutlineThickness(2.f);
            cuadro.setOutlineColor({90, 150, 255, 120});
            m_ventana.draw(cuadro);

            sf::Text ini;
            ini.setFont(m_fuente);
            ini.setString(prog.nombre.empty() ? "?" : std::string(1, prog.nombre[0]));
            ini.setCharacterSize(42);
            ini.setStyle(sf::Text::Bold);
            ini.setFillColor({130, 195, 255});
            centrarTexto(ini, fotoX + fotoSz / 2.f, fotoY + fotoSz / 2.f);
            m_ventana.draw(ini);
        }

        sf::Text tNombre;
        tNombre.setFont(m_fuente); tNombre.setString(prog.nombre);
        tNombre.setCharacterSize(15); tNombre.setStyle(sf::Text::Bold);
        tNombre.setFillColor(sf::Color::White);
        centrarTexto(tNombre, cx, fotoY + fotoSz + 18.f);
        m_ventana.draw(tNombre);

        sf::RectangleShape sepCard({w - 30.f, 1.f});
        sepCard.setPosition(x + 15.f, fotoY + fotoSz + 36.f);
        sepCard.setFillColor({90, 150, 255, 60});
        m_ventana.draw(sepCard);

        sf::Text tCorreo;
        tCorreo.setFont(m_fuente); tCorreo.setString(prog.correo);
        tCorreo.setCharacterSize(11); tCorreo.setFillColor({130, 195, 255});
        centrarTexto(tCorreo, cx, fotoY + fotoSz + 52.f);
        m_ventana.draw(tCorreo);

        sf::Text tRol;
        tRol.setFont(m_fuente); tRol.setString("Programador");
        tRol.setCharacterSize(12); tRol.setFillColor({70, 110, 170});
        centrarTexto(tRol, cx, fotoY + fotoSz + 72.f);
        m_ventana.draw(tRol);
    }

    void centrarTexto(sf::Text& t, float x, float y) {
        sf::FloatRect r = t.getLocalBounds();
        t.setOrigin(r.left + r.width / 2.f, r.top + r.height / 2.f);
        t.setPosition(x, y);
    }

    sf::RenderWindow& m_ventana;
    const sf::Font&   m_fuente;

    std::vector<Programador> m_programadores;
    std::vector<sf::Texture> m_fotos;
    std::vector<bool>        m_fotosCargadas;

    sf::Text m_titulo, m_subtitulo;
    Button   m_btnVolver;
    float    m_tiempo = 0.f;
};
