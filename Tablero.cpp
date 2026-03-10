#include "Tablero.h"
#include <cstdlib> // rand y srand
#include <ctime> // timeee

// Constructor y destructor
Tablero::Tablero(Nivel nivel) : grid(nullptr), banderasColocadas(0), generado(false) {
    configurarNivel(nivel);
    alojarMemoria();
    inicializar();
}

Tablero::~Tablero() {
    liberarMemoria();
}

// Configuración según nivel
void Tablero::configurarNivel(Nivel nivel) {
    switch (nivel) {
        case Nivel::PRINCIPIANTE: {
            filas = 8;
            cols = 8;
            totalMinas = 10;
            break;
        }
        case Nivel::INTERMEDIO: {
            filas = 16;
            cols = 16;
            totalMinas = 40;
            break;
        }
        case Nivel::EXPERTO: {
            filas = 16;
            cols = 30;
            totalMinas = 99;
            break;
        }
    }
}


// Gestión de memoria dinámica
void Tablero::alojarMemoria() {
    // Reservar exactamente filas x columnas celdas sin tamaño fijo
    grid = new Celda*[filas];
    for (int indice = 0; indice < filas; indice++) {
        grid[indice] = new Celda[cols];
    }
}

void Tablero::liberarMemoria() {
    if (grid != nullptr) {
        for (int indice = 0; indice < filas; indice++) {
            delete[] grid[indice];
            grid[indice] = nullptr; // Evitar puntero colgante
        }
        delete[] grid;
        grid = nullptr;
    }
}

// Inicialización
void Tablero::inicializar() {
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < cols; j++) {
            grid[i][j].reiniciar();

            generado = false;
            banderasColocadas = 0;
        }
    }
}

// Generación de minas
// Se llamará en el primer click para asegurar que el jugador nunca pierde al inicio
void Tablero::generarMinas(int filaSegura, int colSegura) {
    srand(static_cast<unsigned int>(time(nullptr)));

    int minasColocadas = 0;
    while (minasColocadas < totalMinas) {
        int f = rand() % filas;
        int c = rand() % cols;


        // No colocar mina en la celda del primer click ni si ya tiene una
        if ((f == filaSegura && c == colSegura) || grid[f][c].getEsMina()) {
            continue;
        }

        grid[f][c].setEsMina(true);
        minasColocadas++;
    }

    calcularVecinos();
    generado = true;
}

// Cálclo de vecinos
void Tablero::calcularVecinos() {
    // 8 direcciones posibles alrededor de una celda
    int direccionF[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int direccionC[] = {-1, 0, 1, -1, 1, -1, 0, 1};

    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < cols; j++) {
            if (grid[i][j].getEsMina()) {
                continue;
            }

            int contador = 0;
            for (int d = 0; d < 8; d++) {
                int ni = i + direccionF[d];
                int nj = j + direccionC[d];
                if (!fueraDeRango(ni, nj) && grid[ni][nj].getEsMina()) {
                    contador++;
                }
            }
            grid[i][j].setMinasVecinas(contador);
        }
    }
}

// Lógica del juego

// Retorna true si el jugador pisó una mina (game over)
bool Tablero::revelarCelda(int fila, int col) {
    if (fueraDeRango(fila, col)) {
        return false;
    }

    if (grid[fila][col].getEstaRevelada()) {
        return false;
    }

    if (grid[fila][col].getTieneBandera()) {
        return false;
    }

    // Primer click: generar minas garantizando celda segura
    if (!generado) {
        generarMinas(fila, col);
    }

    if (grid[fila][col].getEsMina()) {
        grid[fila][col].setEstaRevelada(true);
        return true; // Ha pisado una mina
    }

    // Si es celda vacía (0 vecinos), expandir con Flood Fill
    floodFill(fila, col);
    return false;
}

void Tablero::toggleBandera(int fila, int col) {
    if (fueraDeRango(fila, col)) {
        return;
    }

    if (grid[fila][col].getEstaRevelada()) {
        return;
    }

    bool tenia = grid[fila][col].getTieneBandera();
    grid[fila][col].toggleBandera();

    // Actualizar contador
    if (tenia) {
        banderasColocadas--;
    } else {
        banderasColocadas++;
    }
}

// Flood Fill Recursivo
// Revela en cadena todas las celdas vacías conectadas
void Tablero::floodFill(int fila, int col) {
    // Condiciones de parada: proteccion contra segmentation fault
    if (fueraDeRango(fila, col)) {
        return;
    }

    if (grid[fila][col].getEstaRevelada()) {
        return;
    }

    if (grid[fila][col].getTieneBandera()) {
        return;
    }

    if (grid[fila][col].getEsMina()) {
        return;
    }

    // Revelar la celda actual
    grid[fila][col].setEstaRevelada(true);

    // Si tiene vecinos con minas, para aquí
    if (grid[fila][col].getMinasVecinas() > 0) {
        return;
    }

    // Celda vacía: expandir recursivamente en las 8 direcciones
    int direccionF[] = {-1, -1, -1,  0,  0,  1,  1,  1};
    int direccionC[] = {-1,  0,  1, -1,  1, -1,  0,  1};

    for (int d = 0; d < 8; d++) {
        floodFill(fila + direccionF[d], col + direccionC[d]);
    }
}

// Verificación de victoria
bool Tablero::verificarVictoria() const {
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < cols; j++) {
            // Si hay alguna celda sin revelar que no es mina, aún no ha ganado
            if (!grid[i][j].getEsMina() && !grid[i][j].getEstaRevelada())
                return false;
        }
    }
    return true;
}

// Helpers
bool Tablero::fueraDeRango(int fila, int col) const {
    return (fila < 0 || fila >= filas || col < 0 || col >= cols);
}

Celda* Tablero::getCelda(int fila, int col) const {
    if (fueraDeRango(fila, col)) {
        return nullptr;
    }
    return &grid[fila][col];
}

int  Tablero::getFilas() const {
    return filas;
}

int  Tablero::getCols() const {
    return cols;
}

int  Tablero::getTotalMinas() const {
    return totalMinas;
}

int  Tablero::getBanderasColocadas() const {
    return banderasColocadas;
}

bool Tablero::isGenerado() const {
    return generado;
}
