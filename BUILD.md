# Maze Runner — Build Instructions

## Prerequisites
- C++17 compiler  (g++ 11+ or MSVC 2019+)
- SFML 3.x        (sfml-graphics, sfml-window, sfml-system)
- A TrueType font named `arial.ttf` next to the executable
  (or DejaVu / Liberation Sans — the game tries several paths)

---

## Linux / macOS

### Install SFML 3
```bash
# Ubuntu / Debian
sudo apt install libsfml-dev

# Fedora
sudo dnf install SFML-devel

# macOS (Homebrew)
brew install sfml
```

### Build with g++ directly
```bash
g++ -std=c++17 -O2 \
    main.cpp Game.cpp Grid.cpp Player.cpp Pathfinding.cpp Queue.cpp \
    -lsfml-graphics -lsfml-window -lsfml-system \
    -o MazeRunner
./MazeRunner
```

### Build with CMake
```bash
mkdir build && cd build
cmake ..
make -j4
./MazeRunner
```

---

## Windows (MinGW / MSYS2)
```bash
pacman -S mingw-w64-x86_64-sfml
g++ -std=c++17 -O2 \
    main.cpp Game.cpp Grid.cpp Player.cpp Pathfinding.cpp Queue.cpp \
    -lsfml-graphics -lsfml-window -lsfml-system \
    -o MazeRunner.exe
```

---

## Font
If text doesn't appear, copy any `.ttf` font (e.g. `arial.ttf`,
`DejaVuSans.ttf`) into the same folder as the executable.

---

## Controls
| Key              | Action                          |
|------------------|---------------------------------|
| Arrow Keys / WASD| Move player                     |
| R / Enter / Space| Restart / confirm               |
| V                | Visualize BFS step-by-step      |
| A                | Visualize A* step-by-step       |
| ESC              | Return from visualizer          |

---

## What Changed (v2)

| Issue                    | Fix                                                          |
|--------------------------|--------------------------------------------------------------|
| No start screen          | Full animated title screen with feature list & controls      |
| No end screen            | Rich WIN / LOSE panel with score, stats, and blink prompt    |
| Grid too small           | 21×21 → **31×31** (50% more cells)                          |
| AI always wins           | Move delay 0.15 s → **0.32 s**; repath only every 3 moves   |
| Chase threshold too high | Reduced from 8 → **6** so AI switches modes less aggressively|
| Multiple paths needed    | Extra wall knockouts after DFS give player detour options    |
| Score / stats            | Move counter, score, best session score shown in HUD         |
| WASD support             | Added alongside arrow keys                                   |
| AI distance display      | HUD shows Manhattan distance to AI at all times              |
| Pulsing animations       | Goal cell and AI pulse in real-time; start/end screens animate|
| Player visual            | Bright blue with glow instead of plain red                   |
