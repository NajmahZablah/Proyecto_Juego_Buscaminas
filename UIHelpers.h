#pragma once
#include <SFML/Graphics.hpp>
#include <string>


//  UIHelpers.h
//  Componentes reutilizables de UI para todas las pantallas.
//    Button     → botón con hover y click
//    InputField → campo de texto (soporta contraseña)
//    UI::       → helpers de dibujo

//  BUTTON
//  Botón con 3 estados: normal, hover (mouse encima), press.
//  Uso:
//    btn.setup(font, "texto", posicion, tamaño, tamLetra);
//    btn.update(window);       // en update()
//    btn.draw(window);         // en draw()
//    btn.isClicked(event);     // en handleEvent()
struct Button {
    sf::RectangleShape shape;
    sf::Text           label;

    sf::Color normalColor { 28,  28,  52, 215};
    sf::Color hoverColor  { 55, 115, 195, 235};
    sf::Color pressColor  {185,  70,  35, 235};

    // Configura posición, tamaño y texto del botón
    void setup(const sf::Font& font, const std::string& text,
               sf::Vector2f pos, sf::Vector2f size,
               unsigned int charSize = 20)
    {
        shape.setSize(size);
        shape.setOrigin(size.x / 2.f, size.y / 2.f);
        shape.setPosition(pos);
        shape.setFillColor(normalColor);
        shape.setOutlineThickness(1.8f);
        shape.setOutlineColor({95, 170, 255, 160});

        label.setFont(font);
        label.setString(text);
        label.setCharacterSize(charSize);
        label.setFillColor(sf::Color::White);

        // Centrar el texto dentro del botón
        sf::FloatRect r = label.getLocalBounds();
        label.setOrigin(r.left + r.width / 2.f, r.top + r.height / 2.f);
        label.setPosition(pos);
    }

    // Llama esto cada frame para animación de hover
    void update(const sf::RenderWindow& win) {
        sf::Vector2i mp = sf::Mouse::getPosition(win);
        bool hovered = shape.getGlobalBounds().contains(
            static_cast<float>(mp.x), static_cast<float>(mp.y));
        if (hovered && sf::Mouse::isButtonPressed(sf::Mouse::Left))
            shape.setFillColor(pressColor);
        else if (hovered)
            shape.setFillColor(hoverColor);
        else
            shape.setFillColor(normalColor);
    }

    // Devuelve true si recibió un click (usar en handleEvent)
    bool isClicked(const sf::Event& e) const {
        if (e.type != sf::Event::MouseButtonReleased) return false;
        if (e.mouseButton.button != sf::Mouse::Left)  return false;
        return shape.getGlobalBounds().contains(
            static_cast<float>(e.mouseButton.x),
            static_cast<float>(e.mouseButton.y));
    }

    void draw(sf::RenderWindow& win) {
        win.draw(shape);
        win.draw(label);
    }
};

//  INPUT FIELD
//  Campo de texto interactivo.
//  - Click para enfocar/desenfocar
//  - Escribe caracteres en ascii
//  - Backspace para borrar
//  - isPassword=true muestra *** en lugar del texto
struct InputField {
    sf::RectangleShape box;
    sf::Text           displayText;
    std::string        value;
    bool               focused    = false;
    bool               isPassword = false;
    unsigned int       maxChars   = 20;

    sf::Color idleColor  { 18,  18,  42, 210};
    sf::Color focusColor { 25,  55, 115, 235};

    // pos = centro del campo, size = ancho x alto
    void setup(const sf::Font& font, sf::Vector2f pos, sf::Vector2f size,
               bool pwd = false, unsigned int charSz = 20)
    {
        isPassword = pwd;

        box.setSize(size);
        box.setOrigin(size.x / 2.f, size.y / 2.f);
        box.setPosition(pos);
        box.setFillColor(idleColor);
        box.setOutlineThickness(1.8f);
        box.setOutlineColor({75, 135, 255, 160});

        displayText.setFont(font);
        displayText.setCharacterSize(charSz);
        displayText.setFillColor(sf::Color::White);

        // Texto alineado al borde izquierdo con padding de 14px
        displayText.setPosition(
            pos.x - size.x / 2.f + 14.f,
            pos.y - static_cast<float>(charSz) / 2.f - 1.f);
    }

    // Llama esto en handleEvent() de la pantalla
    void handleEvent(const sf::Event& e) {
        if (e.type == sf::Event::MouseButtonPressed) {
            focused = box.getGlobalBounds().contains(
                static_cast<float>(e.mouseButton.x),
                static_cast<float>(e.mouseButton.y));
            box.setOutlineColor(focused
                                    ? sf::Color{60, 160, 255, 220}
                                    : sf::Color{75, 135, 255, 160});
            box.setFillColor(focused ? focusColor : idleColor);
        }
        if (!focused) return;
        if (e.type == sf::Event::TextEntered) {
            if (e.text.unicode == 8) {
                if (!value.empty()) value.pop_back();
            } else if (e.text.unicode >= 32 && e.text.unicode < 128
                       && value.size() < maxChars) {
                value += static_cast<char>(e.text.unicode);
            }
        }
    }

    // Llama esto en update() para refrescar el texto visible
    void update() {
        std::string shown = isPassword
                                ? std::string(value.size(), '*') : value;
        if (focused) shown += "|";
        displayText.setString(shown);
    }

    void draw(sf::RenderWindow& win) {
        win.draw(box);
        win.draw(displayText);
    }

    void clear() {
        value.clear();
        focused = false;
        box.setFillColor(idleColor);
        box.setOutlineColor({75, 135, 255, 160});
    }
};

//  UI NAMESPACE  –  helpers globales de dibujo
namespace UI {

// Fondo con degradado (otro día con mas tiempo se pone estetic)
inline sf::VertexArray makeGradientBackground(
    unsigned W, unsigned H, sf::Color top, sf::Color bot)
{
    sf::VertexArray bg(sf::TriangleStrip, 4);
    bg[0] = {{0.f,      0.f},      top};
    bg[1] = {{(float)W, 0.f},      top};
    bg[2] = {{0.f,      (float)H}, bot};
    bg[3] = {{(float)W, (float)H}, bot};
    return bg;
}

} // namespace UI
