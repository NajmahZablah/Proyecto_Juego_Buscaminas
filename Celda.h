#ifndef CELDA_H
#define CELDA_H

class Celda {
public:
    // Constructor
    Celda();

    // Getters
    bool getEsMina() const;
    bool getEstaRevelada() const;
    bool getTieneBandera() const;
    int getMinasVecinas() const;

    // Setters
    void setEsMina(bool valor);
    void setEstaRevelada(bool valor);
    void setTieneBandera(bool valor);
    void setMinasVecinas(int cantidad);

    // Acciones
    void toggleBandera();
    void reiniciar();

private:
    bool esMina;
    bool estaRevelada;
    bool tieneBandera;
    int minasVecinas;
};

#endif // CELDA_H
