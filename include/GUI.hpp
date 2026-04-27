#ifndef GUI_HPP
#define GUI_HPP

#include "Tablero.hpp"
#include "ControladorJuego.hpp"
#include <GL/glut.h>
#include <memory>
#include <vector>

/**
 * @brief Clase que gestiona la interfaz gráfica premium del juego.
 * Utiliza OpenGL para renderizado de alta calidad con efectos de neón y glassmorphism.
 */
class GUI {
public:
    /**
     * @brief Inicializa el entorno gráfico.
     */
    static void inicializar(int argc, char** argv, int filas, int cols, ControladorJuego* ctrl);

    /**
     * @brief Bucle principal de eventos de GLUT.
     */
    static void ejecutar();

private:
    // Funciones de callback para GLUT
    static void display();
    static void reshape(int w, int h);
    static void mouse(int button, int state, int x, int y);
    static void keyboard(unsigned char key, int x, int y);
    static void idle();

    // Funciones auxiliares de dibujo
    static void dibujarTablero();
    static void dibujarFondoElegante();
    static void dibujarPieza(int f, int c, int jugador);
    static void dibujarEfectoNeon(float x, float y, float r, float g, float b);

    static int ventanaAncho;
    static int ventanaAlto;
    static int filasTablero;
    static int colsTablero;
    static ControladorJuego* controlador;
    static const Tablero* tableroActual; 
};

#endif // GUI_HPP
