#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "ScreenManager.h"
#include "UIHelpers.h"
#include "UserManager.h"

//  UserAuthScreen.h
//  Pantalla de autenticación con tres modos:
//  [Iniciar Sesion]: usuario + contraseña → LEVEL_SELECT
//  [Crear Cuenta]: usuario + contraseña + confirmación
//  [Eliminar]: usuario + contraseña → borra de users.dat
//  Persistencia: UserManager lee/escribe users.dat con fstream.
//  Los punteros NO se usan aquí; se usan en el tablero del juego.


class UserAuthScreen {
public:

    // Modos de la pantalla
    enum class Tab { LOGIN, REGISTER, DELETE_USER };

    // Constructor
    UserAuthScreen(sf::RenderWindow& win, const sf::Font& font,
                   PlayerData& player)
        : m_win(win), m_font(font), m_player(player) {}

    // onEnter
    // Se llama cada vez que se navega a esta pantalla.
    void onEnter() {
        const float W  = static_cast<float>(m_win.getSize().x);
        const float cx = W / 2.f;

        //  TÍTULO
        m_title.setFont(m_font);
        m_title.setString("ACCESO");
        m_title.setCharacterSize(54);
        m_title.setStyle(sf::Text::Bold);
        m_title.setFillColor(sf::Color::White);
        centerText(m_title, cx, 68.f);

        //  PESTAÑAS  (tres modos)
        //  Ancho: 175px cada una, separadas 4px
        //  Centro en Y=148
        m_tabLogin.setup   (m_font, "Iniciar Sesion",
                         {cx - 185.f, 148.f}, {175.f, 42.f}, 15);
        m_tabRegister.setup(m_font, "Crear Cuenta",
                            {cx,         148.f}, {175.f, 42.f}, 15);
        m_tabDelete.setup  (m_font, "Eliminar",
                          {cx + 185.f, 148.f}, {175.f, 42.f}, 15);

        // Color especial para pestaña Eliminar
        m_tabDelete.normalColor = {70, 15, 15, 200};
        m_tabDelete.hoverColor  = {150, 25, 25, 230};

        //  ETIQUETAS de los campos
        //  Tamaño 15, color azul claro, alineadas a la izquierda
        for (auto* lbl : {&m_lblUser, &m_lblPass, &m_lblConfirm}) {
            lbl->setFont(m_font);
            lbl->setCharacterSize(15);
            lbl->setFillColor({145, 195, 255});
        }

        //  CAMPOS DE TEXTO
        //  Bloque de 72px (etiqueta 20px + gap 6px + campo 46px)
        //  Y_etiqueta = 200, 272, 344
        //  Y_campo    = 222, 294, 366  (centro del campo)
        m_fieldUser.setup   (m_font, {cx, 226.f}, {340.f, 46.f}, false, 20);
        m_fieldPass.setup   (m_font, {cx, 298.f}, {340.f, 46.f}, true,  20);
        m_fieldConfirm.setup(m_font, {cx, 370.f}, {340.f, 46.f}, true,  20);

        for (auto* f : {&m_fieldUser, &m_fieldPass, &m_fieldConfirm})
            f->maxChars = 20;


        //  BOTÓN PRINCIPAL y BOTÓN VOLVER
        //  Sus posiciones Y se ajustan en setTab() según el modo
        m_btnAction.setup(m_font, "ENTRAR",  {cx, 390.f}, {270.f, 50.f}, 21);
        m_btnBack.setup  (m_font, "< Volver",{cx, 458.f}, {200.f, 42.f}, 16);
        m_btnBack.normalColor = {20, 20, 48, 185};
        m_btnBack.hoverColor  = {45, 45, 105, 225};


        //  MENSAJE DE ESTADO
        m_lblMsg.setFont(m_font);
        m_lblMsg.setCharacterSize(14);

        // Empezar en LOGIN y limpiar todo
        setTab(Tab::LOGIN);
        clearMessage();
    }

    // handleEvent
    GameScreen handleEvent(const sf::Event& e) {
        // Eventos al campo de texto activo
        m_fieldUser.handleEvent(e);
        m_fieldPass.handleEvent(e);
        if (m_currentTab == Tab::REGISTER)
            m_fieldConfirm.handleEvent(e);

        // Cambio de pestaña
        if (m_tabLogin.isClicked(e))    setTab(Tab::LOGIN);
        if (m_tabRegister.isClicked(e)) setTab(Tab::REGISTER);
        if (m_tabDelete.isClicked(e))   setTab(Tab::DELETE_USER);

        // Volver al menú
        if (m_btnBack.isClicked(e)) return GameScreen::MAIN_MENU;

        // Acción principal (click o Enter)
        if (m_btnAction.isClicked(e) ||
            (e.type == sf::Event::KeyPressed &&
             e.key.code == sf::Keyboard::Return))
        {
            return processAction();
        }

        return GameScreen::USER_AUTH;
    }

    // update
    void update(float dt) {
        m_tabLogin.update(m_win);
        m_tabRegister.update(m_win);
        m_tabDelete.update(m_win);
        m_btnAction.update(m_win);
        m_btnBack.update(m_win);

        m_fieldUser.update();
        m_fieldPass.update();
        if (m_currentTab == Tab::REGISTER)
            m_fieldConfirm.update();

        // Fade-out del mensaje
        if (m_msgTimer > 0.f) {
            m_msgTimer -= dt;
            sf::Uint8 a = m_msgTimer > 0.5f ? 255 :
                              static_cast<sf::Uint8>(m_msgTimer / 0.5f * 255.f);
            m_lblMsg.setFillColor({m_msgColor.r, m_msgColor.g,
                                   m_msgColor.b, a});
        }
        m_time += dt;
    }

    // draw
    void draw() {
        const float W  = static_cast<float>(m_win.getSize().x);
        const float H  = static_cast<float>(m_win.getSize().y);
        const float cx = W / 2.f;

        // 1. FONDO degradado
        m_win.draw(UI::makeGradientBackground(
            static_cast<unsigned>(W), static_cast<unsigned>(H),
            {4, 7, 22}, {8, 22, 62}));

        // 2. PANEL decorativo (altura dinámica según modo)
        float panelH = (m_currentTab == Tab::REGISTER) ? 400.f : 330.f;
        sf::RectangleShape panel({420.f, panelH});
        panel.setOrigin(210.f, panelH / 2.f);
        panel.setPosition(cx, 138.f + panelH / 2.f);
        panel.setFillColor({255, 255, 255, 10});
        panel.setOutlineThickness(1.4f);
        panel.setOutlineColor({65, 125, 255, 40});
        m_win.draw(panel);

        // 3. TÍTULO
        m_win.draw(m_title);

        // 4. LÍNEA bajo el título
        sf::RectangleShape sep({280.f, 1.f});
        sep.setOrigin(140.f, 0.f);
        sep.setPosition(cx, 103.f);
        sep.setFillColor({50, 95, 195, 90});
        m_win.draw(sep);

        // 5. PESTAÑAS con subrayado en la activa
        drawTab(m_tabLogin,    m_currentTab == Tab::LOGIN,    {65, 180, 255});
        drawTab(m_tabRegister, m_currentTab == Tab::REGISTER, {65, 180, 255});
        drawTab(m_tabDelete,   m_currentTab == Tab::DELETE_USER, {255, 80, 80});

        // 6. BLOQUE  Usuario
        //    Etiqueta a Y=200, campo centrado en Y=226
        m_lblUser.setString("Usuario:");
        m_lblUser.setPosition(cx - 170.f, 200.f);
        m_win.draw(m_lblUser);
        m_fieldUser.draw(m_win);

        // 7. BLOQUE  Contraseña
        //    Etiqueta a Y=272, campo centrado en Y=298
        m_lblPass.setString("Contrasena:");
        m_lblPass.setPosition(cx - 170.f, 272.f);
        m_win.draw(m_lblPass);
        m_fieldPass.draw(m_win);

        // 8. BLOQUE  Confirmar (solo en REGISTRO)
        //    Etiqueta a Y=344, campo centrado en Y=370
        if (m_currentTab == Tab::REGISTER) {
            m_lblConfirm.setString("Confirmar contrasena:");
            m_lblConfirm.setPosition(cx - 170.f, 344.f);
            m_win.draw(m_lblConfirm);
            m_fieldConfirm.draw(m_win);
        }

        // 9. AVISO en modo ELIMINAR
        if (m_currentTab == Tab::DELETE_USER) {
            m_lblConfirm.setString("Se borraran todos tus datos y puntajes.");
            m_lblConfirm.setFillColor({255, 140, 50});
            m_lblConfirm.setPosition(cx - 170.f, 344.f);
            m_win.draw(m_lblConfirm);
            m_lblConfirm.setFillColor({145, 195, 255}); // restaurar color
        }

        // 10. BOTÓN PRINCIPAL
        m_btnAction.draw(m_win);

        // 11. BOTÓN VOLVER
        m_btnBack.draw(m_win);

        // 12. MENSAJE DE ESTADO
        if (m_msgTimer > 0.f) {
            sf::FloatRect mr = m_lblMsg.getLocalBounds();
            m_lblMsg.setOrigin(mr.left + mr.width / 2.f,
                               mr.top  + mr.height / 2.f);
            m_lblMsg.setPosition(cx, 620.f);
            m_win.draw(m_lblMsg);
        }
    }

private:

    // setTab
    // Cambia el modo y reposiciona los botones según
    // cuántos campos hay visibles.
    void setTab(Tab t) {
        m_currentTab = t;
        m_fieldUser.clear();
        m_fieldPass.clear();
        m_fieldConfirm.clear();
        clearMessage();

        float btnY  = 0.f;  // Y del botón principal
        float backY = 0.f;  // Y del botón volver

        switch (t) {
        case Tab::LOGIN:
            // 2 campos: botones más arriba
            m_btnAction.label.setString("ENTRAR");
            m_btnAction.normalColor = {28, 28, 52, 215};
            m_btnAction.hoverColor  = {55, 115, 195, 235};
            btnY  = 390.f;
            backY = 458.f;
            break;

        case Tab::REGISTER:
            // 3 campos:  botones más abajo
            m_btnAction.label.setString("CREAR CUENTA");
            m_btnAction.normalColor = {15, 70, 35, 215};
            m_btnAction.hoverColor  = {25, 140, 65, 235};
            btnY  = 458.f;
            backY = 526.f;
            break;

        case Tab::DELETE_USER:
            // 2 campos + aviso: botones igual que LOGIN
            m_btnAction.label.setString("ELIMINAR CUENTA");
            m_btnAction.normalColor = {80, 15, 15, 215};
            m_btnAction.hoverColor  = {160, 28, 28, 235};
            btnY  = 390.f;
            backY = 458.f;
            break;
        }

        // Reposicionar botones
        m_btnAction.shape.setPosition({m_btnAction.shape.getPosition().x, btnY});
        m_btnAction.label.setPosition({m_btnAction.shape.getPosition().x, btnY});
        m_btnBack.shape.setPosition  ({m_btnBack.shape.getPosition().x,   backY});
        m_btnBack.label.setPosition  ({m_btnBack.shape.getPosition().x,   backY});

        // Recentrar texto del botón principal
        sf::FloatRect r = m_btnAction.label.getLocalBounds();
        m_btnAction.label.setOrigin(r.left + r.width / 2.f,
                                    r.top  + r.height / 2.f);
    }

    // processAction
    // Valida campos y llama a UserManager según el modo activo.
    GameScreen processAction() {
        std::string user = trim(m_fieldUser.value);
        std::string pass = m_fieldPass.value;

        if (user.empty() || pass.empty()) {
            showMessage("Completa todos los campos.", {255, 185, 45});
            return GameScreen::USER_AUTH;
        }

        switch (m_currentTab) {

        // LOGIN
        case Tab::LOGIN:
            if (UserManager::loginUser(user, pass, m_player)) {
                showMessage("Bienvenido, " + user + "!", {55, 215, 95});
                return GameScreen::LEVEL_SELECT;
            }
            showMessage("Usuario o contrasena incorrectos.",
                        {255, 75, 65});
            return GameScreen::USER_AUTH;

        // REGISTRO
        case Tab::REGISTER: {
            std::string confirm = m_fieldConfirm.value;
            if (pass != confirm) {
                showMessage("Las contrasenas no coinciden.",
                            {255, 75, 65});
                return GameScreen::USER_AUTH;
            }
            if (pass.size() < 4) {
                showMessage("Contrasena minima: 4 caracteres.",
                            {255, 185, 45});
                return GameScreen::USER_AUTH;
            }
            if (UserManager::registerUser(user, pass)) {
                UserManager::loadUser(user, m_player);
                showMessage("Cuenta creada! Bienvenido, " + user + "!",
                            {55, 215, 95});
                return GameScreen::LEVEL_SELECT;
            }
            showMessage("Ese nombre ya existe. Elige otro.",
                        {255, 75, 65});
            return GameScreen::USER_AUTH;
        }

        // ELIMINAR
        case Tab::DELETE_USER:
            if (UserManager::deleteUser(user, pass)) {
                // Si era el jugador activo, cerrar su sesión
                if (m_player.username == user)
                    m_player = PlayerData{};
                showMessage("Cuenta eliminada correctamente.",
                            {55, 215, 95});
            } else {
                showMessage("Usuario o contrasena incorrectos.",
                            {255, 75, 65});
            }
            return GameScreen::USER_AUTH;
        }
        return GameScreen::USER_AUTH;
    }

    // Helpers

    void centerText(sf::Text& t, float x, float y) {
        sf::FloatRect r = t.getLocalBounds();
        t.setOrigin(r.left + r.width / 2.f, r.top + r.height / 2.f);
        t.setPosition(x, y);
    }

    // Dibuja una pestaña con subrayado de color si está activa
    void drawTab(Button& tab, bool active, sf::Color lineColor) {
        if (active) {
            sf::RectangleShape line({175.f, 3.f});
            sf::Vector2f tp = tab.shape.getPosition();
            line.setPosition(tp.x - 87.5f, tp.y + 21.f);
            line.setFillColor(lineColor);
            m_win.draw(line);
            tab.shape.setOutlineColor({lineColor.r, lineColor.g,
                                       lineColor.b, 190});
        } else {
            tab.shape.setOutlineColor({42, 60, 100, 80});
        }
        tab.draw(m_win);
    }

    void showMessage(const std::string& msg, sf::Color color) {
        m_msgColor = color;
        m_msgTimer = 3.f;
        m_lblMsg.setString(msg);
        m_lblMsg.setFillColor(color);
    }

    void clearMessage() {
        m_msgTimer = 0.f;
        m_message  = "";
    }

    // Quita espacios al inicio y final
    std::string trim(const std::string& s) {
        std::string r = s;
        while (!r.empty() && r.front() == ' ') r.erase(r.begin());
        while (!r.empty() && r.back()  == ' ') r.pop_back();
        return r;
    }

    //  VARIABLES MIEMBRO

    sf::RenderWindow& m_win;
    const sf::Font&   m_font;
    PlayerData&       m_player;

    Tab        m_currentTab = Tab::LOGIN;

    sf::Text   m_title;       // "ACCESO"
    sf::Text   m_lblUser;     // etiqueta "Usuario:"
    sf::Text   m_lblPass;     // etiqueta "Contrasena:"
    sf::Text   m_lblConfirm;  // etiqueta "Confirmar:" / aviso eliminar
    sf::Text   m_lblMsg;      // mensaje de error o éxito

    Button     m_tabLogin;     // pestaña Iniciar Sesion
    Button     m_tabRegister;  // pestaña Crear Cuenta
    Button     m_tabDelete;    // pestaña Eliminar
    Button     m_btnAction;    // botón principal
    Button     m_btnBack;      // < Volver

    InputField m_fieldUser;    // campo usuario
    InputField m_fieldPass;    // campo contraseña (oculta con ***)
    InputField m_fieldConfirm; // campo confirmar (solo en registro)

    std::string m_message  = "";
    sf::Color   m_msgColor = sf::Color::White;
    float       m_msgTimer = 0.f;
    float       m_time     = 0.f;
};
