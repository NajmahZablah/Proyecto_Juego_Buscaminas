#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "ScreenManager.h"
#include "UIHelpers.h"
#include "UserManager.h"

/*  UserAuthScreen.h
 *  Pantalla de autenticación con tres modos:
 *      [Iniciar Sesion] -> usuario + contraseña -> LEVEL_SELECT
 *      [Crear Cuenta] -> usuario + contraseña + confirmación
 *      [Eliminar] -> usuario + contraseña -> borra de users.dat
 *
 *  Persistencia: UserManager lee/escribe users.dat con fstream
*/

class UserAuthScreen {
public:

    enum class Pestana {
        INICIAR_SESION,
        CREAR_CUENTA,
        ELIMINAR_CUENTA
    };

    UserAuthScreen(sf::RenderWindow& ventana, const sf::Font& fuente, PlayerData& jugador)
        : m_ventana(ventana), m_fuente(fuente), m_jugador(jugador) {}

    void onEnter() {
        const float anchoVentana = static_cast<float>(m_ventana.getSize().x);
        const float centroX = anchoVentana / 2.f;

        // Título
        m_titulo.setFont(m_fuente);
        m_titulo.setString("ACCESO");
        m_titulo.setCharacterSize(54);
        m_titulo.setStyle(sf::Text::Bold);
        m_titulo.setFillColor(sf::Color::White);
        centrarTexto(m_titulo, centroX, 68.f);

        // Pestañas de modo
        m_pestanaIniciar.setup(m_fuente, "Iniciar Sesion", {centroX - 185.f, 148.f}, {175.f, 42.f}, 15);
        m_pestanaCrear.setup  (m_fuente, "Crear Cuenta", {centroX, 148.f}, {175.f, 42.f}, 15);
        m_pestanaEliminar.setup(m_fuente, "Eliminar", {centroX + 185.f, 148.f}, {175.f, 42.f}, 15);
        m_pestanaEliminar.normalColor = {70, 15, 15, 200};
        m_pestanaEliminar.hoverColor = {150, 25, 25, 230};

        // Etiquetas de campos
        for (auto* etiqueta : {&m_etiquetaUsuario, &m_etiquetaContrasena, &m_etiquetaConfirmar}) {
            etiqueta->setFont(m_fuente);
            etiqueta->setCharacterSize(15);
            etiqueta->setFillColor({145, 195, 255});
        }

        // Campos de texto
        m_campoUsuario.setup (m_fuente, {centroX, 226.f}, {340.f, 46.f}, false, 20);
        m_campoContrasena.setup(m_fuente, {centroX, 298.f}, {340.f, 46.f}, true, 20);
        m_campoConfirmar.setup (m_fuente, {centroX, 370.f}, {340.f, 46.f}, true, 20);

        for (auto* campo : {&m_campoUsuario, &m_campoContrasena, &m_campoConfirmar}) {
            campo->maxChars = 20;
        }

        // Botones
        m_btnAccion.setup(m_fuente, "ENTRAR", {centroX, 390.f}, {270.f, 50.f}, 21);
        m_btnVolver.setup(m_fuente, "< Volver",  {centroX, 458.f}, {200.f, 42.f}, 16);
        m_btnVolver.normalColor = {20, 20, 48, 185};
        m_btnVolver.hoverColor = {45, 45, 105, 225};

        // Mensaje de estado
        m_textoMensaje.setFont(m_fuente);
        m_textoMensaje.setCharacterSize(14);

        cambiarPestana(Pestana::INICIAR_SESION);
        limpiarMensaje();
    }

    GameScreen handleEvent(const sf::Event& evento) {
        m_campoUsuario.handleEvent(evento);
        m_campoContrasena.handleEvent(evento);

        if (m_pestanaActual == Pestana::CREAR_CUENTA) {
            m_campoConfirmar.handleEvent(evento);
        }

        // Cambio de pestaña
        if (m_pestanaIniciar.isClicked(evento)) {
            cambiarPestana(Pestana::INICIAR_SESION);
        }
        if (m_pestanaCrear.isClicked(evento)) {
            cambiarPestana(Pestana::CREAR_CUENTA);
        }
        if (m_pestanaEliminar.isClicked(evento)) {
            cambiarPestana(Pestana::ELIMINAR_CUENTA);
        }

        if (m_btnVolver.isClicked(evento)) {
            return GameScreen::MAIN_MENU;
        }

        // Acción principal con click o Enter
        bool clickAccion  = m_btnAccion.isClicked(evento);
        bool enterPresado = (evento.type == sf::Event::KeyPressed && evento.key.code == sf::Keyboard::Return);

        if (clickAccion || enterPresado) {
            return procesarAccion();
        }

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

        if (m_pestanaActual == Pestana::CREAR_CUENTA) {
            m_campoConfirmar.update();
        }

        // Fade-out del mensaje de estado
        if (m_timerMensaje > 0.f) {
            m_timerMensaje -= dt;
            sf::Uint8 alpha = m_timerMensaje > 0.5f ? 255 : static_cast<sf::Uint8>((m_timerMensaje / 0.5f) * 255.f);
            m_textoMensaje.setFillColor({m_colorMensaje.r, m_colorMensaje.g, m_colorMensaje.b, alpha});
        }

        m_tiempo += dt;
    }

    void draw() {
        const float anchoVentana = static_cast<float>(m_ventana.getSize().x);
        const float altoVentana = static_cast<float>(m_ventana.getSize().y);
        const float centroX = anchoVentana / 2.f;

        // Fondo degradado
        m_ventana.draw(UI::makeGradientBackground(static_cast<unsigned>(anchoVentana),
                                                  static_cast<unsigned>(altoVentana),
                                                  {4, 7, 22}, {8, 22, 62}));

        // Panel decorativo
        float altoPanel = (m_pestanaActual == Pestana::CREAR_CUENTA) ? 400.f : 330.f;
        sf::RectangleShape panel({420.f, altoPanel});
        panel.setOrigin(210.f, altoPanel / 2.f);
        panel.setPosition(centroX, 138.f + altoPanel / 2.f);
        panel.setFillColor({255, 255, 255, 10});
        panel.setOutlineThickness(1.4f);
        panel.setOutlineColor({65, 125, 255, 40});
        m_ventana.draw(panel);

        // Título
        m_ventana.draw(m_titulo);

        // Línea bajo el título
        sf::RectangleShape separador({280.f, 1.f});
        separador.setOrigin(140.f, 0.f);
        separador.setPosition(centroX, 103.f);
        separador.setFillColor({50, 95, 195, 90});
        m_ventana.draw(separador);

        // Pestañas con subrayado en la activa
        dibujarPestana(m_pestanaIniciar, m_pestanaActual == Pestana::INICIAR_SESION, {65, 180, 255});
        dibujarPestana(m_pestanaCrear, m_pestanaActual == Pestana::CREAR_CUENTA, {65, 180, 255});
        dibujarPestana(m_pestanaEliminar, m_pestanaActual == Pestana::ELIMINAR_CUENTA, {255, 80, 80});

        // Campo usuario
        m_etiquetaUsuario.setString("Usuario:");
        m_etiquetaUsuario.setPosition(centroX - 170.f, 200.f);
        m_ventana.draw(m_etiquetaUsuario);
        m_campoUsuario.draw(m_ventana);

        // Campo contraseña
        m_etiquetaContrasena.setString("Contrasena:");
        m_etiquetaContrasena.setPosition(centroX - 170.f, 272.f);
        m_ventana.draw(m_etiquetaContrasena);
        m_campoContrasena.draw(m_ventana);

        // Campo confirmar (solo en crear cuenta) o aviso (en eliminar)
        if (m_pestanaActual == Pestana::CREAR_CUENTA) {
            m_etiquetaConfirmar.setString("Confirmar contrasena:");
            m_etiquetaConfirmar.setFillColor({145, 195, 255});
            m_etiquetaConfirmar.setPosition(centroX - 170.f, 344.f);
            m_ventana.draw(m_etiquetaConfirmar);
            m_campoConfirmar.draw(m_ventana);
        }

        if (m_pestanaActual == Pestana::ELIMINAR_CUENTA) {
            m_etiquetaConfirmar.setString("Se borraran todos tus datos y puntajes.");
            m_etiquetaConfirmar.setFillColor({255, 140, 50});
            m_etiquetaConfirmar.setPosition(centroX - 170.f, 344.f);
            m_ventana.draw(m_etiquetaConfirmar);
            m_etiquetaConfirmar.setFillColor({145, 195, 255}); // restaurar
        }

        // Botón principal y volver
        m_btnAccion.draw(m_ventana);
        m_btnVolver.draw(m_ventana);

        // Mensaje de estado con fade-out
        if (m_timerMensaje > 0.f) {
            sf::FloatRect limites = m_textoMensaje.getLocalBounds();
            m_textoMensaje.setOrigin(limites.left + limites.width  / 2.f, limites.top  + limites.height / 2.f);
            m_textoMensaje.setPosition(centroX, 620.f);
            m_ventana.draw(m_textoMensaje);
        }
    }

private:

    // Cambia el modo activo y reposiciona los botones según cuántos campos hay
    void cambiarPestana(Pestana pestana) {
        m_pestanaActual = pestana;
        m_campoUsuario.clear();
        m_campoContrasena.clear();
        m_campoConfirmar.clear();
        limpiarMensaje();

        float posYAccion = 0.f;
        float posYVolver = 0.f;

        switch (pestana) {
        case Pestana::INICIAR_SESION:
            m_btnAccion.label.setString("ENTRAR");
            m_btnAccion.normalColor = {28, 28,  52, 215};
            m_btnAccion.hoverColor = {55, 115, 195, 235};
            posYAccion = 390.f;
            posYVolver = 458.f;
            break;

        case Pestana::CREAR_CUENTA:
            m_btnAccion.label.setString("CREAR CUENTA");
            m_btnAccion.normalColor = {15, 70,  35, 215};
            m_btnAccion.hoverColor = {25, 140, 65, 235};
            posYAccion = 458.f;
            posYVolver = 526.f;
            break;

        case Pestana::ELIMINAR_CUENTA:
            m_btnAccion.label.setString("ELIMINAR CUENTA");
            m_btnAccion.normalColor = {80, 15, 15, 215};
            m_btnAccion.hoverColor = {160, 28, 28, 235};
            posYAccion = 390.f;
            posYVolver = 458.f;
            break;
        }

        m_btnAccion.shape.setPosition({m_btnAccion.shape.getPosition().x, posYAccion});
        m_btnAccion.label.setPosition({m_btnAccion.shape.getPosition().x, posYAccion});
        m_btnVolver.shape.setPosition({m_btnVolver.shape.getPosition().x, posYVolver});
        m_btnVolver.label.setPosition ({m_btnVolver.shape.getPosition().x, posYVolver});

        // Recentrar texto del botón principal
        sf::FloatRect limites = m_btnAccion.label.getLocalBounds();
        m_btnAccion.label.setOrigin(limites.left + limites.width  / 2.f, limites.top  + limites.height / 2.f);
    }

    // Valida campos y llama a UserManager según el modo activo
    GameScreen procesarAccion() {
        std::string nombreUsuario = recortarEspacios(m_campoUsuario.value);
        std::string contrasena = m_campoContrasena.value;

        if (nombreUsuario.empty() || contrasena.empty()) {
            mostrarMensaje("Completa todos los campos.", {255, 185, 45});
            return GameScreen::USER_AUTH;
        }

        switch (m_pestanaActual) {

        case Pestana::INICIAR_SESION:
            if (UserManager::iniciarSesion(nombreUsuario, contrasena, m_jugador)) {
                mostrarMensaje("Bienvenido, " + nombreUsuario + "!", {55, 215, 95});
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
            if (UserManager::registrarUsuario(nombreUsuario, contrasena)) {
                UserManager::cargarUsuario(nombreUsuario, m_jugador);
                mostrarMensaje("Cuenta creada! Bienvenido, " + nombreUsuario + "!",
                               {55, 215, 95});
                return GameScreen::LEVEL_SELECT;
            }
            mostrarMensaje("Ese nombre ya existe. Elige otro.", {255, 75, 65});
            return GameScreen::USER_AUTH;
        }

        case Pestana::ELIMINAR_CUENTA:
            if (UserManager::eliminarUsuario(nombreUsuario, contrasena)) {
                if (m_jugador.nombre == nombreUsuario) {
                    m_jugador = PlayerData{};
                }
                mostrarMensaje("Cuenta eliminada correctamente.", {55, 215, 95});
            } else {
                mostrarMensaje("Usuario o contrasena incorrectos.", {255, 75, 65});
            }
            return GameScreen::USER_AUTH;
        }

        return GameScreen::USER_AUTH;
    }

    // Helpers
    void centrarTexto(sf::Text& texto, float x, float y) {
        sf::FloatRect limites = texto.getLocalBounds();
        texto.setOrigin(limites.left + limites.width  / 2.f, limites.top  + limites.height / 2.f);
        texto.setPosition(x, y);
    }

    void dibujarPestana(Button& boton, bool activa, sf::Color colorLinea) {
        if (activa) {
            sf::RectangleShape subrayado({175.f, 3.f});
            sf::Vector2f posBoton = boton.shape.getPosition();
            subrayado.setPosition(posBoton.x - 87.5f, posBoton.y + 21.f);
            subrayado.setFillColor(colorLinea);
            m_ventana.draw(subrayado);
            boton.shape.setOutlineColor({colorLinea.r, colorLinea.g,
                                         colorLinea.b, 190});
        } else {
            boton.shape.setOutlineColor({42, 60, 100, 80});
        }
        boton.draw(m_ventana);
    }

    void mostrarMensaje(const std::string& mensaje, sf::Color color) {
        m_colorMensaje = color;
        m_timerMensaje = 3.f;
        m_textoMensaje.setString(mensaje);
        m_textoMensaje.setFillColor(color);
    }

    void limpiarMensaje() {
        m_timerMensaje = 0.f;
    }

    std::string recortarEspacios(const std::string& texto) {
        std::string resultado = texto;
        while (!resultado.empty() && resultado.front() == ' ') {
            resultado.erase(resultado.begin());
        }
        while (!resultado.empty() && resultado.back() == ' ') {
            resultado.pop_back();
        }
        return resultado;
    }

    // Variables miembro
    sf::RenderWindow& m_ventana;
    const sf::Font& m_fuente;
    PlayerData& m_jugador;

    Pestana m_pestanaActual = Pestana::INICIAR_SESION;

    sf::Text m_titulo;
    sf::Text m_etiquetaUsuario;
    sf::Text m_etiquetaContrasena;
    sf::Text m_etiquetaConfirmar;
    sf::Text m_textoMensaje;

    Button m_pestanaIniciar;
    Button m_pestanaCrear;
    Button m_pestanaEliminar;
    Button m_btnAccion;
    Button m_btnVolver;

    InputField m_campoUsuario;
    InputField m_campoContrasena;
    InputField m_campoConfirmar;

    sf::Color m_colorMensaje = sf::Color::White;
    float m_timerMensaje = 0.f;
    float m_tiempo = 0.f;
};
