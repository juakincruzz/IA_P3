#include "ControladorJuego.hpp"
#include "AgenteHumanoConsola.hpp"
#include "AgenteEstudiante.hpp"
#include <iostream>

ControladorJuego::ControladorJuego(Tablero tablero, std::shared_ptr<Agente> p1, std::shared_ptr<Agente> p2, double limiteTiempo)
    : tablero(tablero), p1(p1), p2(p2), turnoActual(1), movimientosPendientes(1), ganadorFinal(0), juegoTerminado(false),
      tiempoP1(0), tiempoP2(0), limiteTiempoSegundos(limiteTiempo), busquedaEnCurso(false), sugerenciaEnCurso(false), resolucionEnCurso(false), 
      ultimoResultadoResolucion(""), idH1(1), idH2(1), profundidadGlobal(5) {
    inicioTurno = std::chrono::steady_clock::now();
}

void ControladorJuego::paso() {
    if (juegoTerminado || tablero.estaLleno()) return;

    // DETECCIÓN DE BLOQUEO (Paso de turno automático)
    if (!tablero.tieneMovimientosValidos()) {
        std::cout << "¡Jugador " << turnoActual << " no tiene movimientos válidos! Pasando turno..." << std::endl;
        tablero.pasarTurno(); // Avanzar contador global del tablero (Regla Trinidad)
        turnoActual = (turnoActual == 1) ? 2 : 1; // Cambiar jugador
        movimientosPendientes = 1;
        inicioTurno = std::chrono::steady_clock::now();
        return;
    }

    std::shared_ptr<Agente> agenteActual = (turnoActual == 1) ? p1 : p2;
    
    // Si NO es un agente humano (es una IA), pedimos el movimiento
    if (dynamic_cast<AgenteHumanoConsola*>(agenteActual.get()) == nullptr) {
        
#ifdef USE_THREADS
        if (!busquedaEnCurso) {
            // Iniciar búsqueda asíncrona
            busquedaEnCurso = true;
            futuroMovimiento = std::async(std::launch::async, [this, agenteActual]() {
                return agenteActual->think(this->tablero);
            });
        } else {
            // Verificar si ha terminado
            auto status = futuroMovimiento.wait_for(std::chrono::milliseconds(0));
            if (status == std::future_status::ready) {
                std::pair<int, int> mov = futuroMovimiento.get();
                busquedaEnCurso = false;
                procesarMovimiento(mov.first, mov.second);
            } else {
                // Verificar tiempo límite solo si el agente lo requiere
                if (agenteActual->tieneLimiteDeTiempo() && getTiempoTurnoActual() > limiteTiempoSegundos) {
                    std::cout << "¡Tiempo excedido (" << limiteTiempoSegundos << "s)! Interrumpiendo IA..." << std::endl;
                    agenteActual->interrumpir();
                    // Esperamos a que termine el procesamiento del nivel actual
                    std::pair<int, int> mov = futuroMovimiento.get();
                    busquedaEnCurso = false;
                    procesarMovimiento(mov.first, mov.second);
                }
            }
        }
#else
        // Versión Síncrona (SH)
        std::pair<int, int> mov = agenteActual->think(tablero);
        procesarMovimiento(mov.first, mov.second);
#endif
    }

    // Verificar si hay una sugerencia terminada
    if (sugerenciaEnCurso) {
        auto status = futuroSugerencia.wait_for(std::chrono::milliseconds(0));
        if (status == std::future_status::ready) {
            std::pair<int, int> mov = futuroSugerencia.get();
            sugerenciaEnCurso = false;
            if (mov.first != -1) {
                procesarMovimiento(mov.first, mov.second);
            }
        }
    }

    // Verificar si hay una resolución terminada
    if (resolucionEnCurso) {
        auto status = futuroResolucion.wait_for(std::chrono::milliseconds(0));
        if (status == std::future_status::ready) {
            ultimoResultadoResolucion = futuroResolucion.get();
            resolucionEnCurso = false;
        }
    }
}

bool ControladorJuego::esTurnoDeIA() const {
    std::shared_ptr<Agente> agenteActual = (turnoActual == 1) ? p1 : p2;
    return dynamic_cast<AgenteHumanoConsola*>(agenteActual.get()) == nullptr;
}


void ControladorJuego::actualizarTiempos() {
    auto ahora = std::chrono::steady_clock::now();
    std::chrono::duration<double> transcurrido = ahora - inicioTurno;
    if (turnoActual == 1) tiempoP1 += transcurrido.count();
    else tiempoP2 += transcurrido.count();
    inicioTurno = ahora;
}

double ControladorJuego::getTiempoTurnoActual() const {
    if (juegoTerminado) return 0.0;
    auto ahora = std::chrono::steady_clock::now();
    std::chrono::duration<double> transcurrido = ahora - inicioTurno;
    return transcurrido.count();
}

double ControladorJuego::getTiempoAcumulado(int jugador) const {
    if (jugador == 1) return tiempoP1 + (turnoActual == 1 && !juegoTerminado ? getTiempoTurnoActual() : 0);
    return tiempoP2 + (turnoActual == 2 && !juegoTerminado ? getTiempoTurnoActual() : 0);
}

void ControladorJuego::reiniciar() {
    tablero = Tablero(tablero.getFilas(), tablero.getColumnas(), tablero.getNParaGanar());
    turnoActual = 1;
    movimientosPendientes = 1;
    ganadorFinal = 0;
    juegoTerminado = false;
    tiempoP1 = tiempoP2 = 0;
    inicioTurno = std::chrono::steady_clock::now();
    busquedaEnCurso = false;
    sugerenciaEnCurso = false;
    resolucionEnCurso = false;
    ultimoResultadoResolucion = "";
}

bool ControladorJuego::procesarMovimiento(int f, int c) {
    if (juegoTerminado || tablero.estaLleno()) return false;

    Tablero::TipoCelda tipo = tablero.getTipoCelda(f, c);
    
    // El motor (Tablero) ya se encarga de la lógica de sabotaje y bombas dentro de ponerPieza.
    if (tablero.ponerPieza(f, c, turnoActual)) {
        if (tipo == Tablero::TipoCelda::ROJO) {
            std::cout << "¡CASILLA ROJA! El Jugador " << turnoActual << " se sabotea y coloca una ficha del oponente!" << std::endl;
        } else if (tipo == Tablero::TipoCelda::AMARILLO) {
            std::cout << "¡BOOOOM! Casilla AMARILLA activada en (" << f << ", " << c << "). Fila y columna limpias." << std::endl;
        }
        
        actualizarTiempos();
        std::cout << "Jugador " << turnoActual << " mueve a (" << f << ", " << c << ")" << std::endl;
        
        // Mostrar tablero en consola para feedback dual
        for (int i = 0; i < tablero.getFilas(); ++i) {
            for (int j = 0; j < tablero.getColumnas(); ++j) {
                int celda = tablero.getCelda(i, j);
                if (celda == 0) std::cout << ". ";
                else if (celda == 1) std::cout << "X ";
                else std::cout << "O ";
            }
            std::cout << std::endl;
        }

        int r = tablero.comprobarGanador();
        if (r != 0) {
            ganadorFinal = r;
            juegoTerminado = true;
            if (r == -1) {
                std::cout << "PARTIDA FINALIZADA: Empate!" << std::endl;
            } else {
                std::cout << "PARTIDA FINALIZADA: ¡Gana el Jugador " << r << "!" << std::endl;
            }
        } else {
            if (tablero.estaLleno()) {
                int res = tablero.getGanadorDesempate();
                ganadorFinal = res;
                juegoTerminado = true;
                
                if (res == -1) {
                    std::cout << "PARTIDA FINALIZADA: Empate absoluto (mismas líneas de 5 y 4)." << std::endl;
                } else {
                    std::cout << "PARTIDA FINALIZADA: ¡Gana el Jugador " << res << " por puntos!" << std::endl;
                    std::cout << "Líneas de 5 J1: " << tablero.contarCombinaciones(5, 1) << " | J2: " << tablero.contarCombinaciones(5, 2) << std::endl;
                    std::cout << "Líneas de 4 J1: " << tablero.contarCombinaciones(4, 1) << " | J2: " << tablero.contarCombinaciones(4, 2) << std::endl;
                }
            } else {
                // GESTIÓN DE ESPECIALIDADES
                Tablero::TipoCelda tipo = tablero.getTipoCelda(f, c);
                if (tipo == Tablero::TipoCelda::VERDE) {
                    std::cout << "¡CELDA VERDE! Jugador " << turnoActual << " repite turno." << std::endl;
                    movimientosPendientes = 1; // Se queda en el mismo turno
                } else if (tipo == Tablero::TipoCelda::ROJO) {
                    // Ahora la casilla roja funciona como una normal en términos de turnos,
                    // pero ya ha colocado una pieza enemiga arriba.
                    movimientosPendientes--;
                    if (movimientosPendientes <= 0) {
                        turnoActual = (turnoActual == 1) ? 2 : 1;
                        movimientosPendientes = 1;
                        std::cout << "Siguiente turno: Jugador " << turnoActual << std::endl;
                    }
                } else {
                    movimientosPendientes--;
                    if (movimientosPendientes <= 0) {
                        turnoActual = (turnoActual == 1) ? 2 : 1;
                        movimientosPendientes = 1;
                        std::cout << "Siguiente turno: Jugador " << turnoActual << std::endl;
                    } else {
                        std::cout << "Jugador " << turnoActual << " aún tiene " << movimientosPendientes << " movimiento(s)." << std::endl;
                    }
                }
            }
        }
        return true;
    }
    return false;
}
void ControladorJuego::ejecutarSugerencia() {
    if (juegoTerminado || esTurnoDeIA() || sugerenciaEnCurso) return;
    
    sugerenciaEnCurso = true;
    int hID = (turnoActual == 1) ? idH1 : idH2;
    int idAgente = turnoActual;
    int prof = profundidadGlobal;
    double tMax = limiteTiempoSegundos;
    Tablero tCopia = tablero; // Copiamos el tablero para evitar problemas de hilos
    
    futuroSugerencia = std::async(std::launch::async, [idAgente, prof, tMax, hID, tCopia]() {
        AgenteEstudiante iaTemporal(idAgente, prof, tMax, hID, AgenteEstudiante::ModoJuego::INTELIGENTE);
        return iaTemporal.think(tCopia);
    });
}

void ControladorJuego::ejecutarResolucion() {
    if (juegoTerminado || resolucionEnCurso) return;
    
    resolucionEnCurso = true;
    ultimoResultadoResolucion = "Calculando...";
    
    int idAgente = turnoActual;
    Tablero tCopia = tablero;
    
    futuroResolucion = std::async(std::launch::async, [idAgente, tCopia]() {
        AgenteEstudiante iaTemporal(idAgente, 0, 0.0, 1, AgenteEstudiante::ModoJuego::STATUS);
        std::pair<int, int> mov;
        auto res = iaTemporal.Status(tCopia, mov);
        
        std::string sVer;
        if (res == AgenteEstudiante::Resultado::VICTORIA) sVer = "GANA J1";
        else if (res == AgenteEstudiante::Resultado::DERROTA) sVer = "GANA J2";
        else sVer = "EMPATE";

        // Formatear mensaje con coordenadas: "(f, c) - VEREDICTO"
        return "(" + std::to_string(mov.first) + "," + std::to_string(mov.second) + ") - " + sVer;
    });
}
