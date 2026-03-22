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
 *  ARCHIVO: users.txt (se cambio de .dat a .txt)
 *  FORMATO: nombre,contrasena,mejorFacil,mejorMedio,mejorDificil,mejorCompetitivo
*/

class UserManager {
public:

    // ruta del archivo de usuarios - ahora .txt
    static const std::string RUTA_ARCHIVO;

    // devuelve true si ya existe un usuario con ese nombre
    static bool usuarioExiste(const std::string& nombre) {
        for (auto& jugador : cargarTodos()) {
            if (jugador.nombre == nombre) return true;
        }
        return false;
    }

    // crea un usuario nuevo
    // devuelve false si ya existe o si algun campo esta vacio
    static bool registrarUsuario(const std::string& nombre, const std::string& contrasena) {
        if (nombre.empty() || contrasena.empty()) return false;
        if (usuarioExiste(nombre)) return false;

        std::ofstream archivo(RUTA_ARCHIVO, std::ios::app);
        if (!archivo) return false;

        // formato: nombre,contrasena,facil,medio,dificil,competitivo
        archivo << nombre << "," << contrasena << ","
                << 0 << "," << 0 << "," << 0 << "," << 0 << "\n";
        return true;
    }

    // verifica usuario + contrasena y carga datos si son correctos
    static bool iniciarSesion(const std::string& nombre, const std::string& contrasena,
                              PlayerData& jugador) {
        for (auto& datos : cargarTodos()) {
            if (datos.nombre == nombre && datos.contrasena == contrasena) {
                jugador = datos;
                jugador.sesionActiva = true;
                return true;
            }
        }
        return false;
    }

    // carga los datos de un usuario por nombre
    // util para refrescar datos despues de guardar puntaje
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

    // elimina un usuario si la contrasena es correcta
    static bool eliminarUsuario(const std::string& nombre, const std::string& contrasena) {
        auto lista = cargarTodos();
        auto it = std::find_if(lista.begin(), lista.end(),
                               [&](const PlayerData& j) {
                                   return j.nombre == nombre && j.contrasena == contrasena;
                               });
        if (it == lista.end()) return false;
        lista.erase(it);
        guardarTodos(lista);
        return true;
    }

    // actualiza el mejor puntaje en users.txt
    // solo sobreescribe si el nuevo tiempo es menor (mejor)
    static void guardarPuntaje(const PlayerData& jugador) {
        auto lista = cargarTodos();
        for (auto& datos : lista) {
            if (datos.nombre != jugador.nombre) continue;

            if (jugador.mejorTiempoFacil > 0 &&
                (datos.mejorTiempoFacil == 0 || jugador.mejorTiempoFacil < datos.mejorTiempoFacil))
                datos.mejorTiempoFacil = jugador.mejorTiempoFacil;

            if (jugador.mejorTiempoMedio > 0 &&
                (datos.mejorTiempoMedio == 0 || jugador.mejorTiempoMedio < datos.mejorTiempoMedio))
                datos.mejorTiempoMedio = jugador.mejorTiempoMedio;

            if (jugador.mejorTiempoDificil > 0 &&
                (datos.mejorTiempoDificil == 0 || jugador.mejorTiempoDificil < datos.mejorTiempoDificil))
                datos.mejorTiempoDificil = jugador.mejorTiempoDificil;

            if (jugador.mejorTiempoCompetitivo > 0 &&
                (datos.mejorTiempoCompetitivo == 0 || jugador.mejorTiempoCompetitivo < datos.mejorTiempoCompetitivo))
                datos.mejorTiempoCompetitivo = jugador.mejorTiempoCompetitivo;
        }
        guardarTodos(lista);
    }

    // lee todos los usuarios del archivo y los devuelve como vector
    static std::vector<PlayerData> cargarTodos() {
        std::vector<PlayerData> resultado;
        std::ifstream archivo(RUTA_ARCHIVO);
        if (!archivo) return resultado;

        std::string linea;
        while (std::getline(archivo, linea)) {
            if (linea.empty()) continue;
            std::istringstream flujo(linea);
            std::string campo;
            PlayerData jugador;

            if (std::getline(flujo, campo, ',')) jugador.nombre     = campo;
            if (std::getline(flujo, campo, ',')) jugador.contrasena = campo;
            if (std::getline(flujo, campo, ',')) jugador.mejorTiempoFacil       = std::stoi(campo);
            if (std::getline(flujo, campo, ',')) jugador.mejorTiempoMedio       = std::stoi(campo);
            if (std::getline(flujo, campo, ',')) jugador.mejorTiempoDificil     = campo.empty() ? 0 : std::stoi(campo);
            if (std::getline(flujo, campo, ',')) jugador.mejorTiempoCompetitivo = campo.empty() ? 0 : std::stoi(campo);

            resultado.push_back(jugador);
        }
        return resultado;
    }

private:

    // sobreescribe users.txt con la lista actualizada
    static void guardarTodos(const std::vector<PlayerData>& lista) {
        std::ofstream archivo(RUTA_ARCHIVO, std::ios::trunc);
        for (auto& jugador : lista) {
            archivo << jugador.nombre            << ","
                    << jugador.contrasena        << ","
                    << jugador.mejorTiempoFacil       << ","
                    << jugador.mejorTiempoMedio       << ","
                    << jugador.mejorTiempoDificil     << ","
                    << jugador.mejorTiempoCompetitivo << "\n";
        }
    }
};

// ahora el archivo es .txt en lugar de .dat
const std::string UserManager::RUTA_ARCHIVO = "users.txt";
