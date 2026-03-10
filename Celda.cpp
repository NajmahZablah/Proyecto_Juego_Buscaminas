#include "Celda.h"

// Constructor
Celda::Celda() {
    reiniciar();
}

// Getters
bool Celda::getEsMina() const {
    return esMina;
}

bool Celda::getEstaRevelada() const {
    return estaRevelada;
}

bool Celda::getTieneBandera() const {
    return tieneBandera;
}

int Celda::getMinasVecinas() const {
    return minasVecinas;
}

// Setters
void Celda::setEsMina(bool valor) {
    esMina = valor;
}

void Celda::setEstaRevelada(bool valor) {
    estaRevelada = valor;
}

void Celda::setTieneBandera(bool valor) {
    tieneBandera = valor;
}

void Celda::setMinasVecinas(int cantidad) {
    minasVecinas = cantidad;
}

// Acciones
void Celda::toggleBandera() {
    if (!estaRevelada) {
        tieneBandera = !tieneBandera;
    }
}

// Reinicia todos los atributos a su estado incial
void Celda::reiniciar() {
    esMina = false;
    estaRevelada = false;
    tieneBandera = false;
    minasVecinas = 0;
}
