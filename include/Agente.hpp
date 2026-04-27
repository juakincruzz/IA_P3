#ifndef AGENTE_HPP
#define AGENTE_HPP

#include "Tablero.hpp"
#include <utility>

/**
 * @brief Interfaz base para cualquier agente que participe en el juego.
 */
class Agente {
public:
    virtual ~Agente() {}

    /**
     * @brief Método que debe implementar el agente para decidir su próximo movimiento.
     * @param tablero Estado actual del tablero.
     * @return Par (fila, columna) indicando la casilla elegida.
     */
    virtual std::pair<int, int> think(const Tablero& tablero) = 0;

    /**
     * @brief Indica si el agente debe estar sujeto a límites de tiempo por el controlador.
     */
    virtual bool tieneLimiteDeTiempo() const { return true; }

    /**
     * @brief Solicita al agente que detenga su búsqueda lo antes posible.
     */
    virtual void interrumpir() {}
};

#endif // AGENTE_HPP
