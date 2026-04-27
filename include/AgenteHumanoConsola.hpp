#ifndef AGENTE_HUMANO_CONSOLA_HPP
#define AGENTE_HUMANO_CONSOLA_HPP

#include "Agente.hpp"
#include <iostream>

/**
 * @brief Agente que permite a un humano jugar introduciendo coordenadas por consola.
 */
class AgenteHumanoConsola : public Agente {
public:
    virtual std::pair<int, int> think(const Tablero& tablero) override {
        int f, c;
        std::cout << "Introduce fila y columna (separadas por espacio): ";
        std::cin >> f >> c;
        return {f, c};
    }
};

#endif // AGENTE_HUMANO_CONSOLA_HPP
