#ifndef CONTROLADOR_JUEGO_HPP
#define CONTROLADOR_JUEGO_HPP

#include "Tablero.hpp"
#include "Agente.hpp"
#include <memory>
#include <chrono>
#include <future>
#include <thread>

/**
 * @brief Clase que gestiona el flujo de la partida.
 */
class ControladorJuego {
public:
    ControladorJuego(Tablero tablero, std::shared_ptr<Agente> p1, std::shared_ptr<Agente> p2, double limiteTiempo = 5.0);

    void paso();
    bool procesarMovimiento(int f, int c);
    void reiniciar();
    void ejecutarSugerencia();
    void ejecutarResolucion();

    bool esTurnoDeIA() const;

    const Tablero& getTablero() const { return tablero; }
    int getTurnoActual() const { return turnoActual; }
    int getGanador() const { return ganadorFinal; }
    bool esFinDeJuego() const { return juegoTerminado; }

    // Métodos de tiempo
    double getTiempoTurnoActual() const;
    double getTiempoAcumulado(int jugador) const;

    void setConfig(int h1, int h2, int prof) { idH1 = h1; idH2 = h2; profundidadGlobal = prof; }
    int getHeuristicaID(int jugador) const { return (jugador == 1) ? idH1 : idH2; }
    bool esSugerenciaEnCurso() const { return sugerenciaEnCurso; }
    bool esResolucionEnCurso() const { return resolucionEnCurso; }
    std::string getUltimoResultadoResolucion() const { return ultimoResultadoResolucion; }

private:
    Tablero tablero;
    std::shared_ptr<Agente> p1;
    std::shared_ptr<Agente> p2;
    int turnoActual;     // 1 o 2
    int movimientosPendientes; // Cantidad de movimientos que le quedan al jugador actual
    int ganadorFinal;    // 0: ninguno, 1 o 2: ganador, -1: empate
    bool juegoTerminado;

    // Cronometraje
    std::chrono::time_point<std::chrono::steady_clock> inicioTurno;
    double tiempoP1, tiempoP2;
    double limiteTiempoSegundos;
    void actualizarTiempos();

    // Gestión de Hilos para la IA
    std::future<std::pair<int, int>> futuroMovimiento;
    std::future<std::pair<int, int>> futuroSugerencia;
    std::future<std::string> futuroResolucion;
    bool busquedaEnCurso;
    bool sugerenciaEnCurso;
    bool resolucionEnCurso;
    std::string ultimoResultadoResolucion;
    int idH1, idH2, profundidadGlobal;
};

#endif // CONTROLADOR_JUEGO_HPP
