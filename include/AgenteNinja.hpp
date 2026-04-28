#ifndef AGENTE_NINJA_HPP
#define AGENTE_NINJA_HPP

#include "Agente.hpp"
#include <string>

/**
 * @brief Agente que actúa como puente para conectar con oponentes remotos (Ninjas).
 */
class AgenteNinja : public Agente {
public:
    AgenteNinja(int id, int nivel);
    
    std::pair<int, int> think(const Tablero& tablero) override;
    
    virtual bool tieneLimiteDeTiempo() const override { return false; }
    void interrumpir() override {} // Por ahora síncrono respecto al script

private:
    int id;
    int nivel;
};

#endif // AGENTE_NINJA_HPP
