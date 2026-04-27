#include "Tablero.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>

Tablero Tablero::cargarDesdeFichero(const std::string& ruta) {
    std::ifstream f(ruta);
    if (!f.is_open()) {
        std::cerr << "Error: No se pudo abrir el fichero de tablero " << ruta << std::endl;
        return Tablero(9, 9, 5); // Fallback por defecto
    }

    int fils, cols, nWin;
    f >> fils >> cols >> nWin;
    
    Tablero t(fils, cols, nWin);
    
    int fase, jugador;
    f >> fase >> jugador;

    // Calcular un turnoActual que satisfaga fase y jugador
    // fase = (turno / 2) % 3  =>  turno / 2 = 3*k + fase
    // Si jugador es 1, turno es par: turno = 2 * (3*k + fase)
    // Si jugador es 2, turno es impar: turno = 2 * (3*k + fase) + 1
    // Usamos k=0 para simplificar, pero podríamos ajustarlo según piezas en tablero.
    int turnoCalc = 2 * fase + (jugador - 1);
    t.turnoActual = turnoCalc;

    for (int i = 0; i < fils; ++i) {
        for (int j = 0; j < cols; ++j) {
            int pieza;
            f >> pieza;
            t.rejilla[i][j] = pieza;
        }
    }

    f.close();
    return t;
}

Tablero::Tablero(int filas, int cols, int nParaGanar) 
    : filas(filas), columnas(cols), nParaGanar(nParaGanar), turnoActual(4) {
    // Inicializar la rejilla con ceros (vacío)
    rejilla.assign(filas, std::vector<int>(cols, 0));
    especialidad.assign(filas, std::vector<TipoCelda>(cols, TipoCelda::NORMAL));

    // --- EL DIAMANTE DEL NINJA (Inicio Cuádruple) ---
    // Solo se aplica en el tablero estándar de 9x9
    if (filas == 9 && columnas == 9 && nParaGanar == 5) {
        rejilla[4][0] = 1;
        rejilla[0][4] = 1;
        rejilla[4][8] = 2;
        rejilla[8][4] = 2;
    }

    // --- DISEÑO DEFINITIVO 2026: ANILLOS DE NINJA ---
    // Solo se aplica en el tablero estándar de 9x9
    if (filas == 9 && columnas == 9 && nParaGanar == 5) {
        // 1. Centro y Cruz Central: ROJO (Sabotaje)
        int midF = filas / 2;
        int midC = columnas / 2;
        especialidad[midF][midC] = TipoCelda::ROJO;
        if (midF > 0) especialidad[midF - 1][midC] = TipoCelda::ROJO;
        if (midF < filas - 1) especialidad[midF + 1][midC] = TipoCelda::ROJO;
        if (midC > 0) especialidad[midF][midC - 1] = TipoCelda::ROJO;
        if (midC < columnas - 1) especialidad[midF][midC + 1] = TipoCelda::ROJO;

        // 2. Rombo Exterior: AMARILLO (Bomba)
        if (filas >= 7 && columnas >= 7) {
            if (midF >= 2) especialidad[midF - 2][midC] = TipoCelda::AMARILLO;
            if (midF <= filas - 3) especialidad[midF + 2][midC] = TipoCelda::AMARILLO;
            if (midC >= 2) especialidad[midF][midC - 2] = TipoCelda::AMARILLO;
            if (midC <= columnas - 3) especialidad[midF][midC + 2] = TipoCelda::AMARILLO;
        }

        // 3. Esquinas Interiores: VERDE (Bonus)
        if (filas > 2 && columnas > 2) {
            especialidad[1][1] = TipoCelda::VERDE;
            especialidad[1][columnas - 2] = TipoCelda::VERDE;
            especialidad[filas - 2][1] = TipoCelda::VERDE;
            especialidad[filas - 2][columnas - 2] = TipoCelda::VERDE;
        }
    }
}

bool Tablero::ponerPieza(int f, int c, int jugador) {
    // Verificar si las coordenadas están dentro del tablero
    if (f < 0 || f >= filas || c < 0 || c >= columnas) {
        return false; 
    }
    
    // Verificar si la casilla está vacía
    if (rejilla[f][c] != 0) {
        return false;
    }

    // REGLAS NINJA (Solo en 9x9 con objetivo 5)
    if (filas == 9 && columnas == 9 && nParaGanar == 5) {
        // 1. REGLA DE ADYACENCIA ESTRICTA:
        if (!esVacio() && !tieneAdyacente(f, c)) {
            return false;
        }
        
        // 2. REGLA DE LA TRINIDAD DEL NINJA (f+c)%3
        int ronda = turnoActual / 2;
        int residuoValido = ronda % 3;
        if ((f + c) % 3 != residuoValido) {
            return false;
        }
    }

    // Colocar la pieza según la especialidad de la casilla
    TipoCelda tipo = especialidad[f][c];
    
    if (tipo == TipoCelda::ROJO) {
        // Sabotaje: Se coloca una pieza del oponente
        rejilla[f][c] = (jugador == 1 ? 2 : 1);
    } else if (tipo == Tablero::TipoCelda::AMARILLO) {
        // Bomba: Limpiar fila y columna
        for (int i = 0; i < filas; ++i) rejilla[i][c] = 0;
        for (int j = 0; j < columnas; ++j) rejilla[f][j] = 0;
        
        // La pieza que activa la bomba SOBREVIVE
        rejilla[f][c] = jugador;
        
        // La casilla se vuelve NORMAL tras el uso
        especialidad[f][c] = TipoCelda::NORMAL;
    } else {
        // Normal o Verde: Se coloca la pieza del jugador
        rejilla[f][c] = jugador;
    }
    
    if (tipo != TipoCelda::VERDE) {
        turnoActual++;
    }
    return true;
}

int Tablero::comprobarGanador() {
    lineaGanadora.clear();
    // Direcciones para comprobar: horizontal, vertical, diagonal ascendente, diagonal descendente
    const int df[] = {0, 1, 1, 1};
    const int dc[] = {1, 0, 1, -1};

    for (int f = 0; f < filas; ++f) {
        for (int c = 0; c < columnas; ++c) {
            int jugador = rejilla[f][c];
            if (jugador == 0) continue;

            // Comprobar en las 4 direcciones
            for (int d = 0; d < 4; ++d) {
                int contador = 1;
                std::vector<std::pair<int, int>> posibleLinea = {{f, c}};
                for (int i = 1; i < nParaGanar; ++i) {
                    int nf = f + df[d] * i;
                    int nc = c + dc[d] * i;

                    if (nf >= 0 && nf < filas && nc >= 0 && nc < columnas && rejilla[nf][nc] == jugador) {
                        contador++;
                        posibleLinea.push_back({nf, nc});
                    } else {
                        break;
                    }
                }

                if (contador >= nParaGanar) {
                    lineaGanadora = posibleLinea;
                    return jugador; // Ganador encontrado
                }
            }
        }
    }

    // Ya no devolvemos -1 aquí para permitir que el controlador
    // llene el tablero y luego llame al desempate por puntos.
    return 0; // Sin ganador aún
}

int Tablero::contarCombinaciones(int longitud, int jugador) const {
    int total = 0;
    const int df[] = {0, 1, 1, 1};
    const int dc[] = {1, 0, 1, -1};

    for (int f = 0; f < filas; ++f) {
        for (int c = 0; c < columnas; ++c) {
            for (int d = 0; d < 4; ++d) {
                int contador = 0;
                for (int i = 0; i < longitud; ++i) {
                    int nf = f + df[d] * i;
                    int nc = c + dc[d] * i;
                    if (nf >= 0 && nf < filas && nc >= 0 && nc < columnas && rejilla[nf][nc] == jugador) {
                        contador++;
                    } else {
                        break;
                    }
                }
                if (contador == longitud) total++;
            }
        }
    }
    return total;
}

int Tablero::getGanadorDesempate() const {
    // 1. Criterio 1: Más 5-en-raya
    int p1_5 = contarCombinaciones(5, 1);
    int p2_5 = contarCombinaciones(5, 2);
    if (p1_5 > p2_5) return 1;
    if (p2_5 > p1_5) return 2;

    // 2. Criterio 2: Más 4-en-raya
    int p1_4 = contarCombinaciones(4, 1);
    int p2_4 = contarCombinaciones(4, 2);
    if (p1_4 > p2_4) return 1;
    if (p2_4 > p1_4) return 2;

    // 3. Criterio 3: Más 3-en-raya
    int p1_3 = contarCombinaciones(3, 1);
    int p2_3 = contarCombinaciones(3, 2);
    if (p1_3 > p2_3) return 1;
    if (p2_3 > p1_3) return 2;

    // 4. Empate absoluto
    return -1;
}

std::vector<std::vector<std::pair<int, int>>> Tablero::buscarTodasLasLineas(int longitud, int jugador) const {
    std::vector<std::vector<std::pair<int, int>>> resultados;
    const int df[] = {0, 1, 1, 1};
    const int dc[] = {1, 0, 1, -1};

    for (int f = 0; f < filas; ++f) {
        for (int c = 0; c < columnas; ++c) {
            for (int d = 0; d < 4; ++d) {
                std::vector<std::pair<int, int>> posible;
                int contador = 0;
                for (int i = 0; i < longitud; ++i) {
                    int nf = f + df[d] * i;
                    int nc = c + dc[d] * i;
                    if (nf >= 0 && nf < filas && nc >= 0 && nc < columnas && rejilla[nf][nc] == jugador) {
                        contador++;
                        posible.push_back({nf, nc});
                    } else {
                        break;
                    }
                }
                if (contador == longitud) resultados.push_back(posible);
            }
        }
    }
    return resultados;
}

bool Tablero::estaLleno() const {
    for (int f = 0; f < filas; ++f) {
        for (int c = 0; c < columnas; ++c) {
            if (rejilla[f][c] == 0) {
                return false;
            }
        }
    }
    return true;
}

bool Tablero::esVacio() const {
    for (int f = 0; f < filas; ++f) {
        for (int c = 0; c < columnas; ++c) {
            if (rejilla[f][c] != 0) return false;
        }
    }
    return true;
}

bool Tablero::tieneAdyacente(int f, int c) const {
    for (int df = -1; df <= 1; ++df) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (df == 0 && dc == 0) continue;
            int nf = f + df;
            int nc = c + dc;
            if (nf >= 0 && nf < filas && nc >= 0 && nc < columnas) {
                if (rejilla[nf][nc] != 0) return true;
            }
        }
    }
    return false;
}

bool Tablero::tieneMovimientosValidos() const {
    bool modoNinja = (filas == 9 && columnas == 9 && nParaGanar == 5);
    int residuoValido = modoNinja ? ((turnoActual / 2) % 3) : -1;
    bool vacio = esVacio();

    for (int f = 0; f < filas; ++f) {
        for (int c = 0; c < columnas; ++c) {
            if (rejilla[f][c] == 0) {
                if (modoNinja) {
                    if ((f + c) % 3 == residuoValido && (vacio || tieneAdyacente(f, c))) return true;
                } else {
                    return true; // En modo normal, cualquier casilla vacía vale
                }
            }
        }
    }
    return false;
}
