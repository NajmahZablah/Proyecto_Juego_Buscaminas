#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "ScreenManager.h"
#include "UIHelpers.h"
#include "UserManager.h"

/*  UserAuthScreen.h
 *  Pantalla de autenticacion con tres modos:
 *  [Iniciar Sesion] -> usuario + contrasena -> LEVEL_SELECT
 *  [Crear Cuenta]   -> usuario + contrasena + confirmacion
 *  [Eliminar]       -> usuario + contrasena -> borra de users.txt
*/

class UserAuthScreen {
public:

    enum class Pestana { INICIAR_SESION, CREAR_CUENTA, ELIMINAR_CUENTA };

    UserAuthScreen(sf::RenderWindow& ventana, const sf::Font& fuente, PlayerData& jugador)
        : m_ventana(ventana), m_fuente(fuente), m_jugador(jugador) {}

    void onEnter() {
        const float W  = static_cast<float>(m_ventana.getSize().x);
        const float cx = W / 2.f;

        m_titulo.setFont(m_fuente);
        m_titulo.setString("ACCESO");
        m_titulo.setCharacterSize(54);
        m_titulo.setStyle(sf::Text::Bold);
        m_titulo.setFillColor(sf::Color::White);
        centrarTexto(m_titulo, cx, 60.f);

        // pestanas: 3 botones centrados en la ventana de 600px
        float pW = 160.f;
        m_pestanaIniciar.setup (m_fuente, "Iniciar Sesion", {cx - pW - 5.f, 120.f}, {pW, 38.f}, 13);
        m_pestanaCrear.setup   (m_fuente, "Crear Cuenta",   {cx,            120.f}, {pW, 38.f}, 13);
        m_pestanaEliminar.setup(m_fuente, "Eliminar",       {cx + pW + 5.f, 120.f}, {pW, 38.f}, 13);
        m_pestanaEliminar.normalColor = {70, 15, 15, 200};
        m_pestanaEliminar.hoverColor  = {150, 25, 25, 230};

        // etiquetas
        for (auto* e : {&m_etiquetaUsuario, &m_etiquetaContrasena, &m_etiquetaConfirmar}) {
            e->setFont(m_fuente);
            e->setCharacterSize(13);
            e->setFillColor({145, 195, 255});
        }

        // layout vertical: etiqueta + 4px + caja (44px) + 18px + siguiente etiqueta
        // bloque 1: etiqueta Y=178, caja centrada en Y=200
        // bloque 2: etiqueta Y=258, caja centrada en Y=280
        // bloque 3: etiqueta Y=338, caja centrada en Y=360
        m_campoUsuario.setup   (m_fuente, {cx, 200.f}, {340.f, 42.f}, false, 20);
        m_campoContrasena.setup(m_fuente, {cx, 280.f}, {340.f, 42.f}, true,  20);
        m_campoConfirmar.setup (m_fuente, {cx, 360.f}, {340.f, 42.f}, true,  20);

        for (auto* c : {&m_campoUsuario, &m_campoContrasena, &m_campoConfirmar})
            c->maxChars = 20;

        m_btnAccion.setup(m_fuente, "ENTRAR",   {cx, 380.f}, {260.f, 48.f}, 20);
        m_btnVolver.setup(m_fuente, "< Volver", {cx, 445.f}, {190.f, 40.f}, 15);
        m_btnVolver.normalColor = {20, 20, 48, 185};
        m_btnVolver.hoverColor  = {45, 45, 105, 225};

        m_textoMensaje.setFont(m_fuente);
        m_textoMensaje.setCharacterSize(13);

        cambiarPestana(Pestana::INICIAR_SESION);
        limpiarMensaje();
    }

    GameScreen handleEvent(const sf::Event& evento) {
        m_campoUsuario.handleEvent(evento);
        m_campoContrasena.handleEvent(evento);
        if (m_pestanaActual == Pestana::CREAR_CUENTA)
            m_campoConfirmar.handleEvent(evento);

        if (m_pestanaIniciar.isClicked(evento))  cambiarPestana(Pestana::INICIAR_SESION);
        if (m_pestanaCrear.isClicked(evento))    cambiarPestana(Pestana::CREAR_CUENTA);
        if (m_pestanaEliminar.isClicked(evento)) cambiarPestana(Pestana::ELIMINAR_CUENTA);
        if (m_btnVolver.isClicked(evento))       return GameScreen::MAIN_MENU;

        bool click = m_btnAccion.isClicked(evento);
        bool enter = (evento.type == sf::Event::KeyPressed &&
                      evento.key.code == sf::Keyboard::Return);
        if (click || enter) return procesarAccion();
        return GameScreen::USER_AUTH;
    }

    void update(float dt) {
        m_pestanaIniciar.update(m_ventana);
        m_pestanaCrear.update(m_ventana);
        m_pestanaEliminar.update(m_ventana);
        m_btnAccion.update(m_ventana);
        m_btnVolver.update(m_ventana);
        m_campoUsuario.update();
        m_campoContrasena.update();
        if (m_pestanaActual == Pestana::CREAR_CUENTA)
            m_campoConfirmar.update();

        if (m_timerMensaje > 0.f) {
            m_timerMensaje -= dt;
            sf::Uint8 a = m_timerMensaje > 0.5f
                              ? 255 : static_cast<sf::Uint8>((m_timerMensaje / 0.5f) * 255.f);
            m_textoMensaje.setFillColor({m_colorMensaje.r, m_colorMensaje.g,
                                         m_colorMensaje.b, a});
        }
        m_tiempo += dt;
    }

    void draw() {
        const float W  = static_cast<float>(m_ventana.getSize().x);
        const float H  = static_cast<float>(m_ventana.getSize().y);
        const float cx = W / 2.f;

        m_ventana.draw(UI::makeGradientBackground(
            static_cast<unsigned>(W), static_cast<unsigned>(H),
            {4, 7, 22}, {8, 22, 62}));

        // panel de fondo
        float panelH = (m_pestanaActual == Pestana::CREAR_CUENTA) ? 400.f : 320.f;
        sf::RectangleShape panel({W * 0.82f, panelH});
        panel.setOrigin(panel.getSize().x / 2.f, 0.f);
        panel.setPosition(cx, 155.f);
        panel.setFillColor({255, 255, 255, 10});
        panel.setOutlineThickness(1.2f);
        panel.setOutlineColor({65, 125, 255, 40});
        m_ventana.draw(panel);

        m_ventana.draw(m_titulo);

        sf::RectangleShape sep({W * 0.6f, 1.f});
        sep.setOrigin(sep.getSize().x / 2.f, 0.f);
        sep.setPosition(cx, 92.f);
        sep.setFillColor({50, 95, 195, 90});
        m_ventana.draw(sep);

        dibujarPestana(m_pestanaIniciar,  m_pestanaActual == Pestana::INICIAR_SESION,  {65, 180, 255});
        dibujarPestana(m_pestanaCrear,    m_pestanaActual == Pestana::CREAR_CUENTA,    {65, 180, 255});
        dibujarPestana(m_pestanaEliminar, m_pestanaActual == Pestana::ELIMINAR_CUENTA, {255, 80, 80});

        // bloque 1: etiqueta usuario + caja
        m_etiquetaUsuario.setString("Usuario:");
        m_etiquetaUsuario.setPosition(cx - 170.f, 178.f);
        m_ventana.draw(m_etiquetaUsuario);
        m_campoUsuario.draw(m_ventana);

        // bloque 2: etiqueta contrasena + caja
        m_etiquetaContrasena.setString("Contrasena:");
        m_etiquetaContrasena.setPosition(cx - 170.f, 258.f);
        m_ventana.draw(m_etiquetaContrasena);
        m_campoContrasena.draw(m_ventana);

        // bloque 3 segun pestana
        if (m_pestanaActual == Pestana::CREAR_CUENTA) {
            m_etiquetaConfirmar.setString("Confirmar contrasena:");
            m_etiquetaConfirmar.setFillColor({145, 195, 255});
            m_etiquetaConfirmar.setPosition(cx - 170.f, 338.f);
            m_ventana.draw(m_etiquetaConfirmar);
            m_campoConfirmar.draw(m_ventana);
        }
        if (m_pestanaActual == Pestana::ELIMINAR_CUENTA) {
            m_etiquetaConfirmar.setString("Atencion: se borraran todos tus datos.");
            m_etiquetaConfirmar.setFillColor({255, 140, 50});
            m_etiquetaConfirmar.setPosition(cx - 170.f, 338.f);
            m_ventana.draw(m_etiquetaConfirmar);
            m_etiquetaConfirmar.setFillColor({145, 195, 255});
        }

        m_btnAccion.draw(m_ventana);
        m_btnVolver.draw(m_ventana);

        if (m_timerMensaje > 0.f) {
            sf::FloatRect r = m_textoMensaje.getLocalBounds();
            m_textoMensaje.setOrigin(r.left + r.width / 2.f, r.top + r.height / 2.f);
            m_textoMensaje.setPosition(cx, H - 55.f);
            m_ventana.draw(m_textoMensaje);
        }
    }

private:

    void cambiarPestana(Pestana p) {
        m_pestanaActual = p;
        m_campoUsuario.clear(); m_campoContrasena.clear(); m_campoConfirmar.clear();
        limpiarMensaje();

        float yA = 0.f, yV = 0.f;
        switch (p) {
        case Pestana::INICIAR_SESION:
            m_btnAccion.label.setString("ENTRAR");
            m_btnAccion.normalColor = {28,  28,  52, 215};
            m_btnAccion.hoverColor  = {55, 115, 195, 235};
            yA = 380.f; yV = 445.f; break;
        case Pestana::CREAR_CUENTA:
            m_btnAccion.label.setString("CREAR CUENTA");
            m_btnAccion.normalColor = {15,  70,  35, 215};
            m_btnAccion.hoverColor  = {25, 140,  65, 235};
            yA = 460.f; yV = 525.f; break;
        case Pestana::ELIMINAR_CUENTA:
            m_btnAccion.label.setString("ELIMINAR CUENTA");
            m_btnAccion.normalColor = { 80, 15, 15, 215};
            m_btnAccion.hoverColor  = {160, 28, 28, 235};
            yA = 380.f; yV = 445.f; break;
        }
        float x = m_btnAccion.shape.getPosition().x;
        m_btnAccion.shape.setPosition({x, yA});
        m_btnAccion.label.setPosition({x, yA});
        m_btnVolver.shape.setPosition({m_btnVolver.shape.getPosition().x, yV});
        m_btnVolver.label.setPosition({m_btnVolver.shape.getPosition().x, yV});

        sf::FloatRect r = m_btnAccion.label.getLocalBounds();
        m_btnAccion.label.setOrigin(r.left + r.width / 2.f, r.top + r.height / 2.f);
    }

    GameScreen procesarAccion() {
        std::string nombre    = recortarEspacios(m_campoUsuario.value);
        std::string contrasena = m_campoContrasena.value;

        if (nombre.empty() || contrasena.empty()) {
            mostrarMensaje("Completa todos los campos.", {255, 185, 45});
            return GameScreen::USER_AUTH;
        }
        switch (m_pestanaActual) {
        case Pestana::INICIAR_SESION:
            if (UserManager::iniciarSesion(nombre, contrasena, m_jugador)) {
                mostrarMensaje("Bienvenido, " + nombre + "!", {55, 215, 95});
                return GameScreen::LEVEL_SELECT;
            }
            mostrarMensaje("Usuario o contrasena incorrectos.", {255, 75, 65});
            return GameScreen::USER_AUTH;

        case Pestana::CREAR_CUENTA: {
            std::string confirmar = m_campoConfirmar.value;
            if (contrasena != confirmar) {
                mostrarMensaje("Las contrasenas no coinciden.", {255, 75, 65});
                return GameScreen::USER_AUTH;
            }
            if (contrasena.size() < 4) {
                mostrarMensaje("Contrasena minima: 4 caracteres.", {255, 185, 45});
                return GameScreen::USER_AUTH;
            }
            if (UserManager::registrarUsuario(nombre, contrasena)) {
                UserManager::cargarUsuario(nombre, m_jugador);
                mostrarMensaje("Cuenta creada! Bienvenido, " + nombre + "!", {55, 215, 95});
                return GameScreen::LEVEL_SELECT;
            }
            mostrarMensaje("Ese nombre ya existe. Elige otro.", {255, 75, 65});
            return GameScreen::USER_AUTH;
        }
        case Pestana::ELIMINAR_CUENTA:
            if (UserManager::eliminarUsuario(nombre, contrasena)) {
                if (m_jugador.nombre == nombre) m_jugador = PlayerData{};
                mostrarMensaje("Cuenta eliminada correctamente.", {55, 215, 95});
            } else {
                mostrarMensaje("Usuario o contrasena incorrectos.", {255, 75, 65});
            }
            return GameScreen::USER_AUTH;
        }
        return GameScreen::USER_AUTH;
    }

    void centrarTexto(sf::Text& t, float x, float y) {
        sf::FloatRect r = t.getLocalBounds();
        t.setOrigin(r.left + r.width / 2.f, r.top + r.height / 2.f);
        t.setPosition(x, y);
    }

    void dibujarPestana(Button& boton, bool activa, sf::Color col) {
        if (activa) {
            sf::RectangleShape sub({160.f, 3.f});
            sf::Vector2f pos = boton.shape.getPosition();
            sub.setPosition(pos.x - 80.f, pos.y + 19.f);
            sub.setFillColor(col);
            m_ventana.draw(sub);
            boton.shape.setOutlineColor({col.r, col.g, col.b, 190});
        } else {
            boton.shape.setOutlineColor({42, 60, 100, 80});
        }
        boton.draw(m_ventana);
    }

    void mostrarMensaje(const std::string& msg, sf::Color color) {
        m_colorMensaje = color; m_timerMensaje = 3.f;
        m_textoMensaje.setString(msg); m_textoMensaje.setFillColor(color);
    }
    void limpiarMensaje() { m_timerMensaje = 0.f; }
    std::string recortarEspacios(const std::string& s) {
        std::string r = s;
        while (!r.empty() && r.front() == ' ') r.erase(r.begin());
        while (!r.empty() && r.back()  == ' ') r.pop_back();
        return r;
    }

    sf::RenderWindow& m_ventana;
    const sf::Font&   m_fuente;
    PlayerData&       m_jugador;
    Pestana m_pestanaActual = Pestana::INICIAR_SESION;

    sf::Text m_titulo;
    sf::Text m_etiquetaUsuario, m_etiquetaContrasena, m_etiquetaConfirmar;
    sf::Text m_textoMensaje;
    Button   m_pestanaIniciar, m_pestanaCrear, m_pestanaEliminar;
    Button   m_btnAccion, m_btnVolver;
    InputField m_campoUsuario, m_campoContrasena, m_campoConfirmar;
    sf::Color  m_colorMensaje = sf::Color::White;
    float m_timerMensaje = 0.f;
    float m_tiempo       = 0.f;
};
