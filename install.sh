#!/bin/bash

# Script de instalación para la Práctica 3 - IA (UGR)

echo "Instalando dependencias necesarias (requiere sudo)..."
sudo apt-get update
sudo apt-get install -y build-essential cmake freeglut3-dev libglew-dev libglu1-mesa-dev libxmu-dev libxi-dev python3 python3-requests

echo "Configurando el proyecto con CMake en el directorio raíz..."
cmake .

echo "Compilando el software..."
make -j$(nproc)

echo ""
echo "Instalación completada con éxito."
echo "Puedes ejecutar el juego con: ./n_en_raya"
