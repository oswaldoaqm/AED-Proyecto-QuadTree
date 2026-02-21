cmake_minimum_required(VERSION 3.17)
project(AED-Proyecto-QuadTree)

set(CMAKE_CXX_STANDARD 17)

set(BASE_DIR "${CMAKE_SOURCE_DIR}/codigo")

set(SFML_DIR "${BASE_DIR}/SFML")

include_directories(${SFML_DIR}/include)
include_directories(${BASE_DIR}/include)

link_directories(${SFML_DIR}/lib)

set(SOURCE_FILES
        "${BASE_DIR}/src/main.cpp"
        "${BASE_DIR}/src/Quadtree.cpp"
        codigo/include/Player.h
        codigo/src/Player.cpp
        codigo/include/Enemy.h
        codigo/src/Enemy.cpp
        codigo/include/Projectile.h
        codigo/src/Projectile.cpp
        codigo/include/Particle.h
        codigo/include/PowerUp.h
        codigo/src/PowerUp.cpp
        codigo/include/FloatingText.h
)

add_executable(AED-Proyecto-QuadTree ${SOURCE_FILES})

target_link_libraries(AED-Proyecto-QuadTree sfml-graphics sfml-window sfml-system)