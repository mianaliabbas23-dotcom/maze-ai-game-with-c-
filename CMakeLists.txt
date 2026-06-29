cmake_minimum_required(VERSION 3.16)
project(MazeAIProject)

set(CMAKE_CXX_STANDARD 20)
set(SFML_DIR "D:/cplibraries/SFML/lib/cmake/SFML")

find_package(SFML 3 COMPONENTS Graphics Window System Audio REQUIRED)

add_executable(MazeAIProject
    main.cpp
    Game.cpp
    Grid.cpp
    Player.cpp
    Pathfinding.cpp
    Queue.cpp
)

target_include_directories(MazeAIProject PRIVATE "D:/cplibraries/SFML/include")
target_link_libraries(MazeAIProject SFML::Graphics SFML::Window SFML::System SFML::Audio)

# Copy a font if found on the system
find_file(ARIAL arial.ttf
    PATHS /usr/share/fonts/truetype /usr/share/fonts
    PATH_SUFFIXES msttcorefonts freefont dejavu liberation
    NO_DEFAULT_PATH)

if(ARIAL)
    configure_file(${ARIAL} ${CMAKE_BINARY_DIR}/arial.ttf COPYONLY)
    message(STATUS "Font copied: ${ARIAL}")
else()
    message(WARNING "arial.ttf not found — install fonts-liberation or copy a .ttf manually.")
endif()
