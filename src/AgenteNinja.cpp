#include "AgenteNinja.hpp"
#include "Tablero.hpp"
#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>

AgenteNinja::AgenteNinja(int id, int nivel) : id(id), nivel(nivel) {}

std::pair<int, int> AgenteNinja::think(const Tablero& tablero) {
    std::stringstream ss;
    // Serializamos el estado para el puente
    ss << tablero.getFilas() << " " 
       << tablero.getColumnas() << " " 
       << tablero.getNParaGanar() << " " 
       << id << " " 
       << nivel << " ";
       
    for (int f = 0; f < tablero.getFilas(); ++f) {
        for (int c = 0; c < tablero.getColumnas(); ++c) {
            ss << tablero.getCelda(f, c) << " ";
        }
    }

    std::string comando = "python3 ninja_bridge.py " + ss.str();
    
    char buffer[128];
    std::string resultado = "";
    auto pclose_ptr = [](FILE* f) { pclose(f); };
    std::unique_ptr<FILE, decltype(pclose_ptr)> pipe(popen(comando.c_str(), "r"), pclose_ptr);
    
    if (!pipe) {
        std::cerr << "Error: No se pudo ejecutar el puente ninja_bridge.py" << std::endl;
        return {0, 0}; // Movimiento por defecto en caso de error
    }
    
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        resultado += buffer;
    }

    // El script debe devolver "fila columna"
    int rf, rc;
    std::stringstream rss(resultado);
    if (rss >> rf >> rc) {
        return {rf, rc};
    }

    std::cerr << "Error: Respuesta inválida del Ninja: " << resultado << std::endl;
    return {0, 0};
}
