cmake_minimum_required(VERSION 3.10)
project(NEnRaya)

set(CMAKE_CXX_STANDARD 17)

# Forzar modo Release con optimización -O3
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

include_directories(include Comportamiento_Agente ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})

# Fuentes comunes
set(FUENTES 
    src/main.cpp 
    src/Tablero.cpp 
    src/ControladorJuego.cpp
    src/GUI.cpp
    src/AgenteNinja.cpp
    Comportamiento_Agente/AgenteEstudiante.cpp
)

# Versión con Hebras (Multithreaded)
add_executable(n_en_raya ${FUENTES})
target_compile_definitions(n_en_raya PRIVATE USE_THREADS)
target_link_libraries(n_en_raya ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} pthread)

# Versión Sin Hebras (Single Threaded)
add_executable(n_en_rayaSH ${FUENTES})
target_link_libraries(n_en_rayaSH ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
