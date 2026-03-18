#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "ScreenManager.h"

/*  UserManager.h
 *  Maneja toda la persistencia de usuarios con fstream
 *
 *      ARCHIVO: users.dat (se crea automáticamente)
 *      FORMATO: nombre,contrasena,mejorFacil,mejorMedio,mejorDificil,mejorCompetitivo
 *
 *  FUNCIONES:
 *      usuarioExiste() -> true si el nombre ya está registrado
 *      registrarUsuario() -> crea usuario nuevo con contraseña
 *      iniciarSesion() -> verifica usuario + contraseña y carga datos
 *      cargarUsuario() -> carga datos de un usuario por nombre
 *      eliminarUsuario() -> elimina usuario si la contraseña es correcta
 *      guardarPuntaje() -> actualiza el mejor puntaje del jugador en disco
 *      cargarTodos() -> devuelve todos los usuarios (para el ranking)
*/

class UserManager {
public:

    static const std::string RUTA_ARCHIVO; // "users.dat"

    // Devuelve true si ya existe un usuario con ese nombre
    static bool usuarioExiste(const std::string& nombre) {
        for (auto& jugador : cargarTodos()) {
            if (jugador.nombre == nombre) {
                return true;
            }
        }
        return false;
    }

    // Crea un usuario nuevo
    // Devuelve false si ya existe o si algún campo está vacío
    static bool registrarUsuario(const std::string& nombre, const std::string& contrasena) {
        if (nombre.empty() || contrasena.empty()) {
            return false;
        }

        if (usuarioExiste(nombre)) {
            return false;
        }

        std::ofstream archivo(RUTA_ARCHIVO, std::ios::app);
        if (!archivo) {
            return false;
        }

        // Formato: nombre,contrasena,facil,medio,dificil,competitivo
        archivo << nombre << ","
                << contrasena << ","
                << 0 << ","
                << 0 << ","
                << 0 << ","
                << 0 << "\n";

        return true;
    }

    // Verifica usuario + contraseña. Si son correctos, carga
    // los datos en 'jugador' y devuelve true.
    static bool iniciarSesion(const std::string& nombre, const std::string& contrasena, PlayerData& jugador) {
        for (auto& datos : cargarTodos()) {
            if (datos.nombre == nombre && datos.contrasena == contrasena) {
                jugador = datos;
                jugador.sesionActiva = true;
                return true;
            }
        }
        return false;
    }

    // Carga los datos de un usuario por nombre
    // Útil para refrescar datos después de guardar puntaje
    static bool cargarUsuario(const std::string& nombre, PlayerData& jugador) {
        for (auto& datos : cargarTodos()) {
            if (datos.nombre == nombre) {
                jugador = datos;
                jugador.sesionActiva = true;
                return true;
            }
        }
        return false;
    }

    // Elimina un usuario si la contraseña es correcta
    // Devuelve true si se eliminó correctamente
    static bool eliminarUsuario(const std::string& nombre, const std::string& contrasena) {
        auto lista = cargarTodos();

        auto it = std::find_if(lista.begin(), lista.end(), [&](const PlayerData& jugador) {
            return jugador.nombre == nombre && jugador.contrasena == contrasena;
        });

        if (it == lista.end()) {
            return false;
        }

        lista.erase(it);
        guardarTodos(lista);
        return true;
    }

    /* Actualiza el mejor puntaje del jugador en users.dat
     * Solo sobreescribe si el nuevo tiempo es menor (mejor)
     * Aplica tanto para niveles individuales como para competitivo
    */
    static void guardarPuntaje(const PlayerData& jugador) {
        auto lista = cargarTodos();

        for (auto& datos : lista) {
            if (datos.nombre != jugador.nombre) {
                continue;
            }

            // Nivel fácil
            if (jugador.mejorTiempoFacil > 0 && (datos.mejorTiempoFacil == 0
                                                 || jugador.mejorTiempoFacil < datos.mejorTiempoFacil)) {
                datos.mejorTiempoFacil = jugador.mejorTiempoFacil;
            }

            // Nivel medio
            if (jugador.mejorTiempoMedio > 0 && (datos.mejorTiempoMedio == 0
                                                 || jugador.mejorTiempoMedio < datos.mejorTiempoMedio)) {
                datos.mejorTiempoMedio = jugador.mejorTiempoMedio;
            }

            // Nivel difícil
            if (jugador.mejorTiempoDificil > 0 && (datos.mejorTiempoDificil == 0
                                                   || jugador.mejorTiempoDificil < datos.mejorTiempoDificil)) {
                datos.mejorTiempoDificil = jugador.mejorTiempoDificil;
            }

            // Competitivo (suma de los 3 niveles)
            if (jugador.mejorTiempoCompetitivo > 0 && (datos.mejorTiempoCompetitivo == 0
                                                       || jugador.mejorTiempoCompetitivo < datos.mejorTiempoCompetitivo)) {
                datos.mejorTiempoCompetitivo = jugador.mejorTiempoCompetitivo;
            }
        }

        guardarTodos(lista);
    }

    // Lee todos los usuarios del archivo y los devuelve
    static std::vector<PlayerData> cargarTodos() {
        std::vector<PlayerData> resultado;
        std::ifstream archivo(RUTA_ARCHIVO);

        if (!archivo) {
            return resultado;
        }

        std::string linea;
        while (std::getline(archivo, linea)) {
            if (linea.empty()) {
                continue;
            }

            std::istringstream flujo(linea);
            std::string campo;
            PlayerData jugador;

            if (std::getline(flujo, campo, ',')) jugador.nombre = campo;
            if (std::getline(flujo, campo, ',')) jugador.contrasena = campo;
            if (std::getline(flujo, campo, ',')) jugador.mejorTiempoFacil = std::stoi(campo);
            if (std::getline(flujo, campo, ',')) jugador.mejorTiempoMedio = std::stoi(campo);
            if (std::getline(flujo, campo, ',')) jugador.mejorTiempoDificil = campo.empty() ? 0 : std::stoi(campo);
            if (std::getline(flujo, campo, ',')) jugador.mejorTiempoCompetitivo = campo.empty() ? 0 : std::stoi(campo);

            resultado.push_back(jugador);
        }

        return resultado;
    }

private:

    // Sobrescribe users.dat con la lista actualizada
    static void guardarTodos(const std::vector<PlayerData>& lista) {
        std::ofstream archivo(RUTA_ARCHIVO, std::ios::trunc);

        for (auto& jugador : lista) {
            archivo << jugador.nombre << ","
                    << jugador.contrasena << ","
                    << jugador.mejorTiempoFacil << ","
                    << jugador.mejorTiempoMedio << ","
                    << jugador.mejorTiempoDificil << ","
                    << jugador.mejorTiempoCompetitivo << "\n";
        }
    }
};

const std::string UserManager::RUTA_ARCHIVO = "users.dat";
