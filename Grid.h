#pragma once
#include <vector>
#include "Cell.h"
#include "Config.h"

class Grid
{
public:
    static const int SIZE = ::SIZE;   // pull from Config.h

    Grid();
    void generateMaze();
    Cell& getCell(int x, int y);   // x=col, y=row → cells[y][x]

private:
    std::vector<std::vector<Cell>> cells;
};
