cmake_minimum_required(VERSION 3.17)
project(AED-Proyecto-QuadTree)

# Usamos C++17 como estándar moderno
set(CMAKE_CXX_STANDARD 17)

# =========================================================
# DEFINICIÓN DE RUTAS (Basado en tu imagen)
# =========================================================
# Como tienes todo dentro de la carpeta "codigo", creamos una variable base
set(BASE_DIR "${CMAKE_SOURCE_DIR}/codigo")

# 1. Configuración de SFML
# Tu carpeta SFML está dentro de "codigo"
set(SFML_DIR "${BASE_DIR}/SFML")

# Agregamos los headers de SFML y tus headers propios
include_directories(${SFML_DIR}/include)
include_directories(${BASE_DIR}/include) # Para DataStructures.h, Entity.h, etc.

# Agregamos las librerías de SFML
link_directories(${SFML_DIR}/lib)

# =========================================================
# 2. Definición de Archivos Fuente
# =========================================================
# Listamos los .cpp apuntando a la carpeta correcta
set(SOURCE_FILES
        "${BASE_DIR}/src/main.cpp"      # ¡Asegúrate de haber creado este archivo!
        "${BASE_DIR}/src/Quadtree.cpp"
        codigo/include/Player.h
        codigo/src/Player.cpp
        codigo/include/Enemy.h
        codigo/src/Enemy.cpp
        codigo/include/Projectile.h
        codigo/src/Projectile.cpp
        codigo/include/Particle.h
)

# =========================================================
# 3. Crear Ejecutable
# =========================================================
add_executable(AED-Proyecto-QuadTree ${SOURCE_FILES})

# =========================================================
# 4. Vincular Librerías (Linker)
# =========================================================
target_link_libraries(AED-Proyecto-QuadTree sfml-graphics sfml-window sfml-system)