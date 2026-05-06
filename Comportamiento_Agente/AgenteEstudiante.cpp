#include "AgenteEstudiante.hpp"
#include <iostream>
#include <limits>
#include <vector>
#include <algorithm>
#include <cmath>
#include <functional>

namespace {

int oponenteDe(int jugador) {
    return (jugador == 1) ? 2 : 1;
}

bool esModoCompeticion(const Tablero& tablero) {
    return tablero.getFilas() == 9 && tablero.getColumnas() == 9 && tablero.getNParaGanar() == 5;
}

bool esMovimientoLegalEstimado(const Tablero& tablero, int f, int c) {
    if (tablero.getCelda(f, c) != 0) return false;
    if (!esModoCompeticion(tablero)) return true;
    if ((f + c) % 3 != tablero.getFaseActual() % 3) return false;
    return tablero.esVacio() || tablero.tieneAdyacente(f, c);
}

int contarMovimientosLegalesEstimados(const Tablero& tablero) {
    int total = 0;
    for (int f = 0; f < tablero.getFilas(); ++f) {
        for (int c = 0; c < tablero.getColumnas(); ++c) {
            if (esMovimientoLegalEstimado(tablero, f, c)) ++total;
        }
    }
    return total;
}

double pesoAmenaza(int piezas, int n) {
    if (piezas >= n) return 1000000000.0;
    if (piezas == n - 1) return 220000.0;
    if (piezas == n - 2) return 12000.0;
    if (piezas == n - 3) return 650.0;
    if (piezas == n - 4) return 45.0;
    return 4.0 * piezas;
}

double puntuacionVentana(int propias, int rivales, int n, int rojasVacias, int amarillasVacias) {
    if (propias > 0 && rivales > 0) return 0.0;
    if (propias == 0 && rivales == 0) return 0.0;

    double factorEspecial = 1.0;
    if (rojasVacias > 0) factorEspecial *= 0.45;
    if (amarillasVacias > 0) factorEspecial *= 0.70;

    if (propias > 0) return pesoAmenaza(propias, n) * factorEspecial;
    return -pesoAmenaza(rivales, n) * 1.12 * factorEspecial;
}

double ajusteCasillasEspeciales(const Tablero& tablero, int id) {
    if (!esModoCompeticion(tablero)) return 0.0;

    int jugadorTurno = tablero.getJugadorTurno();
    int rival = oponenteDe(id);
    double score = 0.0;

    for (int f = 0; f < tablero.getFilas(); ++f) {
        for (int c = 0; c < tablero.getColumnas(); ++c) {
            if (!esMovimientoLegalEstimado(tablero, f, c)) continue;

            double signoTurno = (jugadorTurno == id) ? 1.0 : -1.0;
            Tablero::TipoCelda tipo = tablero.getTipoCelda(f, c);

            if (tipo == Tablero::TipoCelda::VERDE) {
                int piezasPropiasCerca = 0;
                int piezasRivalesCerca = 0;
                for (int df = -1; df <= 1; ++df) {
                    for (int dc = -1; dc <= 1; ++dc) {
                        if (df == 0 && dc == 0) continue;
                        int nf = f + df;
                        int nc = c + dc;
                        if (nf < 0 || nf >= tablero.getFilas() || nc < 0 || nc >= tablero.getColumnas()) continue;

                        int celda = tablero.getCelda(nf, nc);
                        if (celda == jugadorTurno) ++piezasPropiasCerca;
                        else if (celda == rival) ++piezasRivalesCerca;
                    }
                }

                score += signoTurno * (1800000.0 + 250000.0 * piezasPropiasCerca - 90000.0 * piezasRivalesCerca);
            } else if (tipo == Tablero::TipoCelda::ROJO) {
                score -= signoTurno * 850000.0;
            } else if (tipo == Tablero::TipoCelda::AMARILLO) {
                int propiasAmenazadas = 0;
                int rivalesAmenazadas = 0;

                for (int i = 0; i < tablero.getFilas(); ++i) {
                    if (i == f) continue;
                    int celda = tablero.getCelda(i, c);
                    if (celda == id) ++propiasAmenazadas;
                    else if (celda == rival) ++rivalesAmenazadas;
                }
                for (int j = 0; j < tablero.getColumnas(); ++j) {
                    if (j == c) continue;
                    int celda = tablero.getCelda(f, j);
                    if (celda == id) ++propiasAmenazadas;
                    else if (celda == rival) ++rivalesAmenazadas;
                }

                score += signoTurno * 220000.0 * (rivalesAmenazadas - propiasAmenazadas);
            }
        }
    }

    return score;
}

double ajusteAmenazasCriticas(const Tablero& tablero, int id) {
    const int rival = oponenteDe(id);
    const int n = tablero.getNParaGanar();
    const int filas = tablero.getFilas();
    const int columnas = tablero.getColumnas();
    const int df[] = {0, 1, 1, 1};
    const int dc[] = {1, 0, 1, -1};
    double score = 0.0;
    int amenazasFuertesPropias = 0;
    int amenazasFuertesRivales = 0;

    for (int f = 0; f < filas; ++f) {
        for (int c = 0; c < columnas; ++c) {
            for (int d = 0; d < 4; ++d) {
                int finF = f + df[d] * (n - 1);
                int finC = c + dc[d] * (n - 1);
                if (finF < 0 || finF >= filas || finC < 0 || finC >= columnas) continue;

                int propias = 0;
                int rivales = 0;
                int vacias = 0;
                int vaciasLegalesAhora = 0;
                int verdesVacias = 0;
                int rojasVacias = 0;
                int amarillasVacias = 0;
                int vaciasCompletablesProximoTurno = 0;
                int verdesCompletablesProximoTurno = 0;

                for (int k = 0; k < n; ++k) {
                    int nf = f + df[d] * k;
                    int nc = c + dc[d] * k;
                    int celda = tablero.getCelda(nf, nc);

                    if (celda == id) {
                        ++propias;
                    } else if (celda == rival) {
                        ++rivales;
                    } else {
                        ++vacias;
                        if (esMovimientoLegalEstimado(tablero, nf, nc)) ++vaciasLegalesAhora;

                        Tablero::TipoCelda tipo = tablero.getTipoCelda(nf, nc);
                        if (tipo == Tablero::TipoCelda::VERDE) ++verdesVacias;
                        else if (tipo == Tablero::TipoCelda::ROJO) ++rojasVacias;
                        else if (tipo == Tablero::TipoCelda::AMARILLO) ++amarillasVacias;
                    }
                }

                if (propias > 0 && rivales > 0) continue;
                if (propias == 0 && rivales == 0) continue;

                int jugadorLinea = (propias > 0) ? id : rival;
                int piezas = (propias > 0) ? propias : rivales;
                double signo = (jugadorLinea == id) ? 1.0 : -1.0;
                double valor = 0.0;

                int faseProximoTurno = tablero.getFaseActual();
                int movimientosProximoTurno = tablero.getMovimientosRestantes();
                if (tablero.getJugadorTurno() != jugadorLinea) {
                    faseProximoTurno += 1;
                    movimientosProximoTurno = esModoCompeticion(tablero) ? 2 : 1;
                }

                for (int k = 0; k < n; ++k) {
                    int nf = f + df[d] * k;
                    int nc = c + dc[d] * k;
                    if (tablero.getCelda(nf, nc) != 0) continue;

                    Tablero::TipoCelda tipo = tablero.getTipoCelda(nf, nc);
                    if (tipo == Tablero::TipoCelda::ROJO || tipo == Tablero::TipoCelda::AMARILLO) continue;

                    bool jugableEnFase = !esModoCompeticion(tablero) ||
                        ((nf + nc) % 3 == faseProximoTurno % 3 && tablero.tieneAdyacente(nf, nc));
                    if (jugableEnFase) {
                        ++vaciasCompletablesProximoTurno;
                        if (tipo == Tablero::TipoCelda::VERDE) ++verdesCompletablesProximoTurno;
                    }
                }

                if (piezas == n - 1) valor = 6500000.0;
                else if (piezas == n - 2) valor = 850000.0;
                else if (piezas == n - 3) valor = 75000.0;
                else if (piezas == n - 4) valor = 5000.0;

                if (valor == 0.0) continue;

                if (piezas >= n - 2) {
                    if (jugadorLinea == id) ++amenazasFuertesPropias;
                    else ++amenazasFuertesRivales;
                }

                if (jugadorLinea != id) valor *= 2.35;
                if (verdesVacias > 0) valor *= (jugadorLinea == id) ? 1.15 : 1.45;
                if (rojasVacias > 0) valor *= 0.35;
                if (amarillasVacias > 0) valor *= 0.65;

                if (tablero.getJugadorTurno() == jugadorLinea) {
                    int piezasQuePuedePoner = std::min(tablero.getMovimientosRestantes(), vaciasLegalesAhora);
                    if (piezas + piezasQuePuedePoner >= n) {
                        valor += 500000000.0;
                    }
                }

                int movimientosEfectivosProximoTurno = movimientosProximoTurno;
                if (verdesCompletablesProximoTurno > 0) ++movimientosEfectivosProximoTurno;
                int piezasQuePuedeCompletar = std::min(movimientosEfectivosProximoTurno, vaciasCompletablesProximoTurno);
                if (piezas + piezasQuePuedeCompletar >= n) {
                    valor += 900000000.0;
                }

                score += signo * valor;
            }
        }
    }

    score += 220000.0 * amenazasFuertesPropias * amenazasFuertesPropias;
    score -= 850000.0 * amenazasFuertesRivales * amenazasFuertesRivales;

    return score;
}

}

AgenteEstudiante::AgenteEstudiante(int id, int profundidadMax, double tiempoMax, int numHeuristica, ModoJuego modo) 
    : id(id), profundidadMax(profundidadMax), tiempoMaxSegundos(tiempoMax), numHeuristica(numHeuristica), modo(modo), abortarBanda(false) {
    nodosVisitados = 0;
}

bool AgenteEstudiante::tieneLimiteDeTiempo() const {
    return modo != ModoJuego::STATUS;
}

std::pair<int, int> AgenteEstudiante::think(const Tablero& tablero) {
    std::pair<int, int> mejor;
    nodosVisitados = 0;
    abortarBanda = false;
    inicioBusqueda = std::chrono::steady_clock::now();

    switch (modo)
    {
    case ModoJuego::ALEATORIO:
        return JuegaAleatorio(tablero);
        break;
    
    case ModoJuego::STATUS:
        Status(tablero, mejor);
        return mejor;
        break;    

    case ModoJuego::MINIMAX:
        minimax(tablero, 0, profundidadMax, mejor);
        return mejor;
        break; 

    case ModoJuego::INTELIGENTE:
        return JuegaInteligente(tablero);   
        break;
    }
        
    return {-1, -1};
}


/**
    * @brief Compara dos tableros para identificar cuál ha sido el movimiento realizado.
    * @param padre Estado inicial del tablero.
    * @param hijo Estado resultante tras un movimiento.
    * @return Un par (fila, columna) con la posición de la nueva pieza.
*/
std::pair<int, int> SacarMovimiento(const Tablero& padre, const Tablero &hijo){
    for(int f=0; f<padre.getFilas(); ++f)
        for(int c=0; c<padre.getColumnas(); ++c)
            if (padre.getCelda(f,c) == 0 && hijo.getCelda(f,c) != 0) 
                return {f, c};
    return {-1, -1};
}

/**
    * @brief Implementa un agente que juega de forma totalmente aleatoria.
    * @param tablero Estado actual del juego.
    * @return La jugada elegida al azar.
*/
std::pair<int, int> AgenteEstudiante::JuegaAleatorio(const Tablero& tablero) {

    // Calculo los tableros descendientes de tablero
    auto sucesores = tablero.getSucesores();

    // Si no tiene descendientes, paso el turno
    if (sucesores.empty()) return {-1, -1};

    // Elijo aleatoriamente uno de los descendientes
    int elegido = rand() % sucesores.size();

    // Saco el movimiento realizado comparando el tablero original con el elegido.
    std::pair<int,int> Mov = SacarMovimiento(tablero, sucesores[elegido]);

    return Mov;
}


/**
    * @brief Algoritmo de resolución completa para estados de final de juego.
    * Determina si una posición está matemáticamente ganada, perdida o empatada.
    * @param tablero Estado a evaluar.
    * @param Mov [Salida] La jugada óptima encontrada.
    * @return Resultado del análisis (VICTORIA, DERROTA o EMPATE).
*/
AgenteEstudiante::Resultado AgenteEstudiante::Status(const Tablero &tablero, std::pair<int,int> &Mov) {
    /* ============== Este trozo de código se tiene que quedar aquí  =============== */
    nodosVisitados++;
    /* ============== Empieza a partir de aquí tu implementación  =============== */

    int ganador = tablero.comprobarGanador();
    if (ganador == id) {
        Mov = {-1, -1};
        return Resultado::VICTORIA;
    }
    if (ganador == oponenteDe(id)) {
        Mov = {-1, -1};
        return Resultado::DERROTA;
    }
    if (ganador == -1) {
        Mov = {-1, -1};
        return Resultado::EMPATE;
    }

    auto sucesores = tablero.getSucesoresConMovimientos();
    if (sucesores.empty()) {
        Mov = {-1, -1};
        return Resultado::EMPATE;
    }

    bool esTurnoAgente = (tablero.getJugadorTurno() == id);
    Resultado mejorResultado = esTurnoAgente ? Resultado::DERROTA : Resultado::VICTORIA;
    Mov = sucesores.front().second;

    for (const auto &sucesor : sucesores) {
        std::pair<int, int> movHijo;
        Resultado resultadoHijo = Status(sucesor.first, movHijo);

        if (esTurnoAgente) {
            if (resultadoHijo == Resultado::VICTORIA) {
                Mov = sucesor.second;
                return Resultado::VICTORIA;
            }
            if (resultadoHijo == Resultado::EMPATE && mejorResultado == Resultado::DERROTA) {
                mejorResultado = Resultado::EMPATE;
                Mov = sucesor.second;
            }
        } else {
            if (resultadoHijo == Resultado::DERROTA) {
                Mov = sucesor.second;
                return Resultado::DERROTA;
            }
            if (resultadoHijo == Resultado::EMPATE && mejorResultado == Resultado::VICTORIA) {
                mejorResultado = Resultado::EMPATE;
                Mov = sucesor.second;
            }
        }
    }

    return mejorResultado;
}



/**
    * @brief Implementación del algoritmo Minimax clásico.
    * @param tablero Estado actual.
    * @param profundidad Nivel actual en el árbol de búsqueda.
    * @param prof_Max Límite de profundidad de la búsqueda.
    * @param Mov [Salida] La mejor jugada encontrada en la raíz.
    * @return Valor heurístico del estado.
*/
double AgenteEstudiante::minimax(const Tablero &tablero, int profundidad, int prof_Max, std::pair<int,int> &Mov) {
    /* ============== Este trozo de código se tiene que quedar aquí  =============== */
    nodosVisitados++;
    if (abortarBanda) return 0;
    
    if (std::chrono::duration<double>(std::chrono::steady_clock::now() - inicioBusqueda).count() > tiempoMaxSegundos) {
        abortarBanda = true;
        return 0;
    }
    /* ============== Empieza a partir de aquí tu implementación  =============== */

    int ganador = tablero.comprobarGanador();
    if (ganador == id) {
        Mov = {-1, -1};
        return GANAR - profundidad;
    }
    if (ganador == oponenteDe(id)) {
        Mov = {-1, -1};
        return PERDER + profundidad;
    }
    if (ganador == -1) {
        Mov = {-1, -1};
        return 0;
    }

    if (profundidad >= prof_Max) {
        Mov = {-1, -1};
        return heuristica(tablero);
    }

    auto sucesores = tablero.getSucesoresConMovimientos();
    if (sucesores.empty()) {
        Mov = {-1, -1};
        return heuristica(tablero);
    }

    bool esTurnoAgente = (tablero.getJugadorTurno() == id);
    double mejorValor = esTurnoAgente ? MenosInfinito : MasInfinito;
    Mov = sucesores.front().second;

    for (const auto &sucesor : sucesores) {
        std::pair<int, int> movHijo;
        double valorHijo = minimax(sucesor.first, profundidad + 1, prof_Max, movHijo);

        if (abortarBanda) return 0;

        if (esTurnoAgente) {
            if (valorHijo > mejorValor) {
                mejorValor = valorHijo;
                Mov = sucesor.second;
            }
        } else {
            if (valorHijo < mejorValor) {
                mejorValor = valorHijo;
                Mov = sucesor.second;
            }
        }
    }

    return mejorValor;
}


/**
    * @brief Punto de entrada para el juego inteligente.
    * @param tablero Estado actual del juego.
    * @return La jugada elegida por el algoritmo de búsqueda.
*/
std::pair<int, int> AgenteEstudiante::JuegaInteligente(const Tablero& tablero) {
    std::pair<int,int> Mov;

    double valor = alfaBeta(tablero, 0, profundidadMax, MenosInfinito, MasInfinito, Mov);
    std::cout << "Valor Minimax: " << valor << "\tJugada: (" << Mov.first << ", " << Mov.second << ")\n";
    return Mov;
}




/**
    * @brief Implementación del algoritmo Minimax con Poda Alfa-Beta.
    * @param tablero Estado actual.
    * @param profundidad Nivel actual en el árbol de búsqueda.
    * @param prof_Max Límite de profundidad de la búsqueda.
    * @param alfa Valor mínimo garantizado para el jugador MAX.
    * @param beta Valor máximo garantizado para el jugador MIN.
    * @param Mov [Salida] La mejor jugada encontrada en la raíz.
    * @return Valor heurístico del estado tras la poda.
*/
double AgenteEstudiante::alfaBeta(const Tablero &tablero, int profundidad, int prof_Max, double alfa, double beta, std::pair<int,int> &Mov) {
    /* ============== Este trozo de código se tiene que quedar aquí  =============== */
    nodosVisitados++;
    if (abortarBanda) return 0;
    
    if (std::chrono::duration<double>(std::chrono::steady_clock::now() - inicioBusqueda).count() > tiempoMaxSegundos) {
        abortarBanda = true;
        return 0;
    }
    /* ============== Empieza a partir de aquí tu implementación  =============== */

    int ganador = tablero.comprobarGanador();
    if (ganador == id) {
        Mov = {-1, -1};
        return GANAR - profundidad;
    }
    if (ganador == oponenteDe(id)) {
        Mov = {-1, -1};
        return PERDER + profundidad;
    }
    if (ganador == -1) {
        Mov = {-1, -1};
        return 0;
    }

    if (profundidad >= prof_Max) {
        Mov = {-1, -1};
        return heuristica(tablero);
    }

    auto sucesores = tablero.getSucesoresConMovimientos();
    if (sucesores.empty()) {
        Mov = {-1, -1};
        return heuristica(tablero);
    }

    bool esTurnoAgente = (tablero.getJugadorTurno() == id);
    double mejorValor = esTurnoAgente ? MenosInfinito : MasInfinito;
    Mov = sucesores.front().second;

    auto actualizaMejor = [&](double valorHijo, const std::pair<int, int> &movimiento) {
        if (esTurnoAgente) {
            if (valorHijo > mejorValor) {
                mejorValor = valorHijo;
                Mov = movimiento;
            }
            alfa = std::max(alfa, mejorValor);
        } else {
            if (valorHijo < mejorValor) {
                mejorValor = valorHijo;
                Mov = movimiento;
            }
            beta = std::min(beta, mejorValor);
        }
        return alfa >= beta;
    };

    if (profundidad + 1 < prof_Max && sucesores.size() > 1) {
        struct SucesorOrdenado {
            double valor;
            Tablero tablero;
            std::pair<int, int> movimiento;
        };

        std::vector<SucesorOrdenado> ordenados;
        ordenados.reserve(sucesores.size());
        for (const auto &sucesor : sucesores) {
            ordenados.push_back({heuristica(sucesor.first), sucesor.first, sucesor.second});
        }

        std::stable_sort(ordenados.begin(), ordenados.end(),
            [esTurnoAgente](const SucesorOrdenado &a, const SucesorOrdenado &b) {
                return esTurnoAgente ? a.valor > b.valor : a.valor < b.valor;
            });

        for (const auto &sucesor : ordenados) {
            std::pair<int, int> movHijo;
            double valorHijo = alfaBeta(sucesor.tablero, profundidad + 1, prof_Max, alfa, beta, movHijo);

            if (abortarBanda) return 0;
            if (actualizaMejor(valorHijo, sucesor.movimiento)) break;
        }
    } else {
        for (const auto &sucesor : sucesores) {
            std::pair<int, int> movHijo;
            double valorHijo = alfaBeta(sucesor.first, profundidad + 1, prof_Max, alfa, beta, movHijo);

            if (abortarBanda) return 0;
            if (actualizaMejor(valorHijo, sucesor.second)) break;
        }
    }

    return mejorValor;
}

/**
    * @brief Función heurística para evaluar la calidad de un tablero.
    * @param tablero Estado a evaluar.
    * @return Puntuación numérica (positiva para ventaja de J1, negativa para J2).
*/
double AgenteEstudiante::heuristica(const Tablero& tablero) {
    switch(numHeuristica) {
        case 0: return heuristicaPrueba(tablero);
                break;
        case 1: return heuristica1(tablero);
                break;
        case 2: return heuristica2(tablero);
                break;
        default: return heuristica1(tablero);
    }
}

double AgenteEstudiante::heuristicaPrueba(const Tablero& tablero) {
    // n es el número de fichas en línea para ganar.
    int n = tablero.getNParaGanar();
    int oponente = (id == 1) ? 2 : 1;
    double score_positivo = 0;

    double score_negativo = 0;

    for (int f=0; f< tablero.getFilas(); f++ ){
        for (int c = 0; c< tablero.getColumnas(); c++){
            if (tablero.getCelda(f,c) != 0 ){
                int valor = tablero.getFilas()-abs(f-(tablero.getFilas()/2)) + tablero.getColumnas()-abs(c-(tablero.getColumnas()/2)); 
                if (tablero.getCelda(f,c) == id){
                    score_positivo += valor;
                }
                else {
                    score_negativo += valor;
                }
            }
        }
    }


    return score_positivo - score_negativo;
}


double AgenteEstudiante::heuristica1(const Tablero& tablero) {
    int ganador = tablero.comprobarGanador();
    if (ganador == id) return GANAR;
    if (ganador == oponenteDe(id)) return PERDER;
    if (ganador == -1) return 0.0;

    const int rival = oponenteDe(id);
    const int n = tablero.getNParaGanar();
    const int filas = tablero.getFilas();
    const int columnas = tablero.getColumnas();
    const int df[] = {0, 1, 1, 1};
    const int dc[] = {1, 0, 1, -1};

    double score = 0.0;

    for (int f = 0; f < filas; ++f) {
        for (int c = 0; c < columnas; ++c) {
            int celda = tablero.getCelda(f, c);
            if (celda == 0) continue;

            int valorCentro = (filas - std::abs(f - filas / 2)) + (columnas - std::abs(c - columnas / 2));
            if (celda == id) {
                score += 16.0 + 7.0 * valorCentro;
            } else {
                score -= 16.0 + 7.0 * valorCentro;
            }
        }
    }

    for (int f = 0; f < filas; ++f) {
        for (int c = 0; c < columnas; ++c) {
            for (int d = 0; d < 4; ++d) {
                int finF = f + df[d] * (n - 1);
                int finC = c + dc[d] * (n - 1);
                if (finF < 0 || finF >= filas || finC < 0 || finC >= columnas) continue;

                int propias = 0;
                int rivales = 0;
                int rojasVacias = 0;
                int amarillasVacias = 0;

                for (int k = 0; k < n; ++k) {
                    int nf = f + df[d] * k;
                    int nc = c + dc[d] * k;
                    int celda = tablero.getCelda(nf, nc);

                    if (celda == id) {
                        ++propias;
                    } else if (celda == rival) {
                        ++rivales;
                    } else {
                        Tablero::TipoCelda tipo = tablero.getTipoCelda(nf, nc);
                        if (tipo == Tablero::TipoCelda::ROJO) ++rojasVacias;
                        else if (tipo == Tablero::TipoCelda::AMARILLO) ++amarillasVacias;
                    }
                }

                score += puntuacionVentana(propias, rivales, n, rojasVacias, amarillasVacias);
            }
        }
    }

    int lineas4Propias = tablero.contarCombinaciones(std::max(1, n - 1), id);
    int lineas4Rivales = tablero.contarCombinaciones(std::max(1, n - 1), rival);
    int lineas3Propias = tablero.contarCombinaciones(std::max(1, n - 2), id);
    int lineas3Rivales = tablero.contarCombinaciones(std::max(1, n - 2), rival);

    score += 18000.0 * (lineas4Propias - 1.15 * lineas4Rivales);
    score += 900.0 * (lineas3Propias - 1.10 * lineas3Rivales);

    int movilidad = contarMovimientosLegalesEstimados(tablero);
    if (tablero.getJugadorTurno() == id) {
        score += 12.0 * movilidad;
    } else {
        score -= 12.0 * movilidad;
    }

    score += ajusteCasillasEspeciales(tablero, id);
    score += ajusteAmenazasCriticas(tablero, id);

    return score;
}

double AgenteEstudiante::heuristica2(const Tablero& tablero) {
    int ganador = tablero.comprobarGanador();
    if (ganador == id) return GANAR;
    if (ganador == oponenteDe(id)) return PERDER;
    if (ganador == -1) return 0.0;

    int rival = oponenteDe(id);
    int n = tablero.getNParaGanar();

    double score = heuristicaPrueba(tablero);
    score += 50000.0 * (tablero.contarCombinaciones(std::max(1, n - 1), id) - tablero.contarCombinaciones(std::max(1, n - 1), rival));
    score += 2500.0 * (tablero.contarCombinaciones(std::max(1, n - 2), id) - tablero.contarCombinaciones(std::max(1, n - 2), rival));
    score += 120.0 * (tablero.contarCombinaciones(std::max(1, n - 3), id) - tablero.contarCombinaciones(std::max(1, n - 3), rival));

    return score;
}
