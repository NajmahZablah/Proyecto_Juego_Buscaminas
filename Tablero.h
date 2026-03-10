#ifndef TABLERO_H
#define TABLERO_H

#include "Celda.h"

// Niveles de dificultad
enum class Nivel {
    PRINCIPIANTE, // 8x8 - 10 minas
    INTERMEDIO, // 16x16 - 40 minas
    EXPERTO // 16x30 - 99 minas
};

class Tablero {
public:
    // Constructor y Destructor (libera memoria dinámica)
    Tablero(Nivel nivel);
    ~Tablero();

    // Inicialización
    void inicializar();
    void generarMinas(int filaSegura, int colSegura);
    void calcularVecinos();

    // Lógica del juego
    // Retorna true si el jugador pisó una mina
    bool revelarCelda(int fila, int col);
    void toggleBandera(int fila, int col);

    // Verificación
    bool verificarVictoria() const;
    bool fueraDeRango(int fila, int col) const;

    // Getters del estado
    Celda* getCelda(int fila, int col) const;
    int getFilas() const;
    int getCols() const;
    int getTotalMinas() const;
    int getBanderasColocadas() const;
    bool isGenerado() const;

private:
    Celda** grid; // Matriz dinámica de punteros
    int filas;
    int cols;
    int totalMinas;
    int banderasColocadas;
    bool generado;

    // Flood Fill recursivo: revela celdas vacías en cadena
    void floodFill(int fila, int col);

    // Configura dimensiones y minas según el nivel
    void configurarNivel(Nivel nivel);

    // Alojamiento y liberación de memoria
    void alojarMemoria();
    void liberarMemoria();
};

#endif // TABLERO_H
