#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "ScreenManager.h"

//  UserManager.h
//  Maneja toda la persistencia de usuarios con fstream.
//
//  ARCHIVO: users.dat  (se crea automáticamente)
//  FORMATO: username,password,bestEasy,bestMed,bestHard
//
//  FUNCIONES:
//    userExists(): true si el nombre ya está registrado
//    registerUser(): crea usuario nuevo con contraseña
//    loginUser(): verifica usuario + contraseña
//    loadUser(): carga datos de un usuario por nombre
//    deleteUser(): elimina usuario si contraseña correcta
//    saveScore(): guarda el mejor puntaje del jugador
//    loadAll(): devuelve todos los usuarios (ranking)

class UserManager {
public:

    static const std::string FILE_PATH; // "users.dat"

    // userExists
    // Devuelve true si ya existe un usuario con ese nombre.
    static bool userExists(const std::string& username) {
        for (auto& u : loadAll())
            if (u.username == username) return true;
        return false;
    }

    // registerUser
    // Crea un usuario nuevo. Devuelve false si ya existe
    // o si algún campo está vacío.
    static bool registerUser(const std::string& username,
                             const std::string& password) {
        if (username.empty() || password.empty()) return false;
        if (userExists(username)) return false;
        std::ofstream ofs(FILE_PATH, std::ios::app);
        if (!ofs) return false;
        ofs << username << "," << password << ",0,0,0\n";
        return true;
    }

    // loginUser
    // Verifica usuario + contraseña. Si son correctos carga
    // los datos en 'out' y devuelve true.
    static bool loginUser(const std::string& username,
                          const std::string& password,
                          PlayerData& out) {
        for (auto& u : loadAll()) {
            if (u.username == username && u.password == password) {
                out          = u;
                out.loggedIn = true;
                return true;
            }
        }
        return false;
    }

    // loadUser
    // Carga los datos de un usuario por nombre.
    // Útil para refrescar datos después de guardar puntaje.
    static bool loadUser(const std::string& username, PlayerData& out) {
        for (auto& u : loadAll()) {
            if (u.username == username) {
                out          = u;
                out.loggedIn = true;
                return true;
            }
        }
        return false;
    }

    // deleteUser
    // Elimina un usuario si la contraseña es correcta.
    // Devuelve true si se eliminó correctamente.
    static bool deleteUser(const std::string& username,
                           const std::string& password) {
        auto users = loadAll();
        auto it = std::find_if(users.begin(), users.end(),
                               [&](const PlayerData& u) {
                                   return u.username == username && u.password == password;
                               });
        if (it == users.end()) return false;
        users.erase(it);
        saveAll(users);
        return true;
    }

    // saveScore
    // Actualiza el mejor puntaje del jugador en users.dat.
    // Solo sobreescribe si el nuevo tiempo es menor (mejor).
    static void saveScore(const PlayerData& player) {
        auto users = loadAll();
        for (auto& u : users) {
            if (u.username != player.username) continue;
            if (player.bestScoreEasy > 0 &&
                (u.bestScoreEasy == 0 ||
                 player.bestScoreEasy < u.bestScoreEasy))
                u.bestScoreEasy = player.bestScoreEasy;
            if (player.bestScoreMed > 0 &&
                (u.bestScoreMed == 0 ||
                 player.bestScoreMed < u.bestScoreMed))
                u.bestScoreMed = player.bestScoreMed;
            if (player.bestScoreHard > 0 &&
                (u.bestScoreHard == 0 ||
                 player.bestScoreHard < u.bestScoreHard))
                u.bestScoreHard = player.bestScoreHard;
        }
        saveAll(users);
    }

    // loadAll
    // Lee todos los usuarios del archivo y los devuelve.
    static std::vector<PlayerData> loadAll() {
        std::vector<PlayerData> result;
        std::ifstream ifs(FILE_PATH);
        if (!ifs) return result;
        std::string line;
        while (std::getline(ifs, line)) {
            if (line.empty()) continue;
            std::istringstream ss(line);
            std::string tok;
            PlayerData p;
            if (std::getline(ss, tok, ',')) p.username      = tok;
            if (std::getline(ss, tok, ',')) p.password      = tok;
            if (std::getline(ss, tok, ',')) p.bestScoreEasy = std::stoi(tok);
            if (std::getline(ss, tok, ',')) p.bestScoreMed  = std::stoi(tok);
            if (std::getline(ss, tok, ',')) p.bestScoreHard = std::stoi(tok);
            result.push_back(p);
        }
        return result;
    }

private:
    // saveAll
    // Sobrescribe users.dat con la lista actualizada.
    static void saveAll(const std::vector<PlayerData>& users) {
        std::ofstream ofs(FILE_PATH, std::ios::trunc);
        for (auto& u : users)
            ofs << u.username      << ","
                << u.password      << ","
                << u.bestScoreEasy << ","
                << u.bestScoreMed  << ","
                << u.bestScoreHard << "\n";
    }
};

const std::string UserManager::FILE_PATH = "users.dat";
