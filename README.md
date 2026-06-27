# Práctica 3 - Inteligencia Artificial

![C++](https://img.shields.io/badge/C++-17-blue)
![AI](https://img.shields.io/badge/Artificial%20Intelligence-Game%20AI-purple)
![Minimax](https://img.shields.io/badge/Search-Minimax-orange)
![Alpha--Beta](https://img.shields.io/badge/Pruning-Alpha--Beta-green)
![Grade](https://img.shields.io/badge/Grade-10%2F10-brightgreen)

Game-playing AI agent developed in C++ for the subject **Inteligencia Artificial** at the **University of Granada**.

The project implements an intelligent agent for a variant of *n en raya*, using adversarial search algorithms, custom heuristics and Alpha-Beta pruning to make competitive decisions in a game environment.

---

## Project Summary

| Area            | Description                         |
| --------------- | ----------------------------------- |
| Subject         | Inteligencia Artificial             |
| Language        | C++                                 |
| Problem type    | Game AI / adversarial search        |
| Main algorithms | Status, Minimax, Alpha-Beta pruning |
| Main heuristic  | Heuristic 1                         |
| Final result    | 8/8 victories against ninjas        |
| Final grade     | 10/10                               |

---

## Description

La práctica consiste en diseñar un agente inteligente que tome decisiones en un juego de tablero. Para ello se han implementado algoritmos clásicos de búsqueda en juegos adversarios, junto con heurísticas que permiten valorar posiciones cuando no es posible explorar todo el árbol de juego.

El agente evalúa el tablero desde su propia perspectiva:

* Valor positivo: posición favorable para el agente.
* Valor negativo: posición favorable para el rival.
* Valor cero: posición equilibrada o empate.

## Technical highlights

This project focuses on several important Game AI concepts:

* Implementation of adversarial search algorithms.
* Full game-tree exploration with `Status` for small boards.
* Depth-limited `Minimax` with heuristic evaluation.
* `Alpha-Beta pruning` to reduce the number of explored branches.
* Heuristic ordering of successors to improve pruning efficiency.
* Custom evaluation function adapted to competition mode.
* Detection of critical threats that can be completed in the current or next turn.
* Evaluation of special cells and multi-move turns.
* Internal helper functions hidden inside an anonymous namespace to keep the public interface clean.



## Algoritmos implementados

### Status

Explora de forma exhaustiva el árbol de juego, sin límite de profundidad y sin utilizar heurística.

Devuelve:

* Victoria
* Derrota
* Empate

Este algoritmo se utiliza principalmente para tableros pequeños, donde sí es posible analizar todos los estados posibles.

### Minimax

Implementación del algoritmo **Minimax** con límite de profundidad.

Cuando se alcanza un estado terminal, devuelve victoria, derrota o empate. Si se alcanza la profundidad máxima, se evalúa el tablero mediante la heurística seleccionada.

### Alfa-Beta

Optimización de Minimax mediante **poda Alfa-Beta**.

Permite descartar ramas que no pueden modificar la decisión final, lo que hace posible alcanzar mayor profundidad de búsqueda. Además, los sucesores se ordenan previamente usando la heurística para mejorar la poda.

## Heurísticas

### Heurística 0

Heurística de prueba basada principalmente en el valor posicional de las fichas, favoreciendo las posiciones cercanas al centro del tablero.

Se mantiene como referencia para comparar con otras heurísticas.

### Heurística 1

Heurística principal utilizada en las pruebas finales y en el modo competición.

Combina:

* Detección de estados terminales.
* Valor posicional de las fichas.
* Evaluación de líneas parciales.
* Penalización de amenazas fuertes del rival.
* Recompensa de amenazas propias.
* Movilidad disponible.
* Tratamiento de casillas especiales.
* Consideración de la fase Trinidad en modo competición.

Esta heurística intenta detectar no solo las amenazas actuales, sino también aquellas que pueden completarse en el turno actual o en el siguiente, especialmente teniendo en cuenta que en el modo competición pueden jugarse varios movimientos por turno.

### Heurística 2

Heurística alternativa más simple.

Combina la heurística de prueba con recuentos de combinaciones parciales. No se utiliza como heurística final, pero sirve como comparación intermedia entre la heurística básica y la heurística principal.

## Resultados

Se realizaron pruebas contra los ninjas usando la heurística 1.

### Minimax con heurística 1

Se probó el algoritmo Minimax con profundidad 4, tanto como jugador 1 como jugador 2.

| Rol del agente | Rival  | Resultado |
| -------------- | ------ | --------- |
| Jugador 1      | ninja1 | Victoria  |
| Jugador 1      | ninja2 | Victoria  |
| Jugador 1      | ninja3 | Victoria  |
| Jugador 1      | ninja4 | Victoria  |
| Jugador 2      | ninja1 | Victoria  |
| Jugador 2      | ninja2 | Victoria  |
| Jugador 2      | ninja3 | Victoria  |
| Jugador 2      | ninja4 | Victoria  |

### Alfa-Beta con heurística 1

Se probó el algoritmo Alfa-Beta con profundidad 7, usando el modo inteligente.

| Rol del agente | Rival  | Resultado |
| -------------- | ------ | --------- |
| Jugador 1      | ninja1 | Victoria  |
| Jugador 1      | ninja2 | Victoria  |
| Jugador 1      | ninja3 | Victoria  |
| Jugador 1      | ninja4 | Victoria  |
| Jugador 2      | ninja1 | Victoria  |
| Jugador 2      | ninja2 | Victoria  |
| Jugador 2      | ninja3 | Victoria  |
| Jugador 2      | ninja4 | Victoria  |

### Resumen de resultados

| Algoritmo | Heurística | Profundidad | Victorias contra ninjas |
| --------- | ---------: | ----------: | ----------------------: |
| Minimax   |          1 |           4 |                     8/8 |
| Alfa-Beta |          1 |           7 |                     8/8 |

La poda Alfa-Beta obtuvo los mismos resultados que Minimax en las pruebas realizadas, pero permitió alcanzar mayor profundidad de búsqueda.

La práctica obtuvo una calificación final de **10/10**, lo que confirma el buen funcionamiento de la heurística diseñada y de los algoritmos implementados.

## Organización del código

Los métodos principales del agente están implementados en:

```text
AgenteEstudiante.hpp
AgenteEstudiante.cpp
```

Además de los métodos declarados en la clase `AgenteEstudiante`, se han añadido funciones auxiliares en un `namespace` anónimo dentro de `AgenteEstudiante.cpp`.

Estas funciones se usan para:

* Estimar movimientos legales.
* Valorar amenazas.
* Detectar el modo competición.
* Evaluar casillas especiales.
* Calcular movilidad.
* Reforzar amenazas críticas.

Se han dejado en el `.cpp` porque son detalles internos de implementación y no forman parte de la interfaz pública del agente. De esta forma, el fichero de cabecera mantiene únicamente la interfaz de la clase `AgenteEstudiante`, mientras que el `.cpp` contiene la implementación concreta y las funciones auxiliares privadas.

## Compilación

Para instalar dependencias y compilar por primera vez:

```bash
./install.sh
```

Para recompilar después de realizar cambios:

```bash
make -j$(nproc)
```

## Ejecución

Ejecutar con hebras:

```bash
./n_en_raya
```

Ejecutar sin hebras:

```bash
./n_en_rayaSH
```

Ejemplo de partida entre dos jugadores humanos:

```bash
./n_en_raya -p1 humano -p2 humano
```

## Conclusión

La parte más importante de la práctica ha sido el diseño de una buena función de evaluación. En el modo competición no basta con contar fichas alineadas, ya que las reglas de fase, los turnos con varios movimientos y las casillas especiales modifican mucho el valor real de cada posición.

La heurística 1 ha sido la más efectiva porque combina evaluación posicional, amenazas, movilidad y reglas específicas del modo competición. Gracias a esta estrategia, el agente consiguió ganar todas las pruebas realizadas contra los ninjas y la práctica obtuvo una calificación final de **10/10**.
