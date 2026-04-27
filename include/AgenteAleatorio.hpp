#ifndef AGENTE_ALEATORIO_HPP
#define AGENTE_ALEATORIO_HPP

#include "Agente.hpp"
#include <vector>
#include <cstdlib>
#include <ctime>

/**
 * @brief Agente que realiza movimientos de forma aleatoria.
 */
class AgenteAleatorio : public Agente {
public:
    AgenteAleatorio() {
        std::srand(std::time(0));
    }

    std::pair<int, int> think(const Tablero& tablero) override {
        std::vector<std::pair<int, int>> disponibles;
        for (int f = 0; f < tablero.getFilas(); ++f) {
            for (int c = 0; c < tablero.getColumnas(); ++c) {
                if (tablero.getCelda(f, c) == 0) {
                    disponibles.push_back({f, c});
                }
            }
        }
        if (disponibles.empty()) return {-1, -1};
        return disponibles[std::rand() % disponibles.size()];
    }
};

#endif // AGENTE_ALEATORIO_HPP
