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
        std::vector<Tablero> sucesores = tablero.getSucesores();
        if (sucesores.empty()) return {-1, -1};

        // Elegimos un sucesor al azar
        const Tablero& elegido = sucesores[std::rand() % sucesores.size()];

        // Para devolver la coordenada (f, c), tenemos que buscar qué celda ha cambiado
        // ya que el motor espera coordenadas.
        for (int f = 0; f < tablero.getFilas(); ++f) {
            for (int c = 0; c < tablero.getColumnas(); ++c) {
                if (tablero.getCelda(f, c) != elegido.getCelda(f, c)) {
                    return {f, c};
                }
            }
        }

        return {-1, -1};
    }
};

#endif // AGENTE_ALEATORIO_HPP
