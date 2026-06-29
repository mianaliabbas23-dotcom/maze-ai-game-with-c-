#pragma once

struct Cell
{
    int x, y;
    bool isWall;

    Cell(int x = 0, int y = 0)
    {
        this->x = x;
        this->y = y;
        isWall  = false;
    }
};
