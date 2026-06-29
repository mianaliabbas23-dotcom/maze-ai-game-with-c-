#include "Grid.h"
#include "Stack.h"
#include "Config.h"
#include <cstdlib>
#include <ctime> //  pattern avoid krny ka lia time pass krty ha 

Grid::Grid()
{
    std::srand((unsigned)std::time(nullptr));

    cells.resize(SIZE, std::vector<Cell>(SIZE));
    for (int row = 0; row < SIZE; row++)
        for (int col = 0; col < SIZE; col++)
            cells[row][col] = Cell(col, row);

    generateMaze();
}

Cell& Grid::getCell(int x, int y)
{
    return cells[y][x];
}

// ═══════════════════════════════════════════════════════════
//  DFS Maze Generation  (custom Stack, DSA requirement)
//  Moves in steps of 2 to carve passages.
//  After generation, extra walls are randomly removed to
//  create multiple paths — giving the player detour options
//  to evade the AI.
// ═══════════════════════════════════════════════════════════
void Grid::generateMaze()
{
    // Mark everything as wall
    for (int row = 0; row < SIZE; row++)
        for (int col = 0; col < SIZE; col++)
            cells[row][col].isWall = true;

    int dx[4] = {  0,  0,  2, -2 };
    int dy[4] = {  2, -2,  0,  0 };

    bool visited[SIZE][SIZE] = {};

    Stack stk;
    stk.push(0, 0);
    visited[0][0]      = true;
    cells[0][0].isWall = false;

    while (!stk.isEmpty())
    {
        int cx = stk.peekX();
        int cy = stk.peekY();

        // Fisher-Yates shuffle of 4 directions
        int dirs[4] = { 0, 1, 2, 3 };
        for (int i = 3; i > 0; i--)
        {
            int j   = rand() % (i + 1);
            int tmp = dirs[i]; dirs[i] = dirs[j]; dirs[j] = tmp;
        }

        bool moved = false;
        for (int i = 0; i < 4; i++)
        {
            int d  = dirs[i];
            int nx = cx + dx[d];
            int ny = cy + dy[d];

            if (nx >= 0 && ny >= 0 && nx < SIZE && ny < SIZE && !visited[ny][nx])
            {
                visited[ny][nx] = true;
                int mx = (cx + nx) / 2;
                int my = (cy + ny) / 2;
                cells[my][mx].isWall = false;
                cells[ny][nx].isWall = false;
                stk.push(nx, ny);
                moved = true;
                break;
            }
        }
        if (!moved) stk.pop();
    }

    // Guarantee start and goal are open
    cells[0][0].isWall           = false;
    cells[SIZE-1][SIZE-1].isWall = false;

    // ── Extra shortcuts: knock out ~12% of interior walls ──
    // This creates multiple viable paths so the player can
    // dodge the AI by taking an alternate route.
    int extraKnockouts = (SIZE * SIZE) / 8;
    for (int k = 0; k < extraKnockouts; k++)
    {
        int row = 1 + rand() % (SIZE - 2);
        int col = 1 + rand() % (SIZE - 2);
        // Only remove walls surrounded by at least 2 open neighbours
        if (cells[row][col].isWall)
        {
            int openNeighbours = 0;
            if (row > 0        && !cells[row-1][col].isWall) openNeighbours++;
            if (row < SIZE-1   && !cells[row+1][col].isWall) openNeighbours++;
            if (col > 0        && !cells[row][col-1].isWall) openNeighbours++;
            if (col < SIZE-1   && !cells[row][col+1].isWall) openNeighbours++;
            if (openNeighbours >= 2)
                cells[row][col].isWall = false;
        }
    }

    // Final guarantee
    cells[0][0].isWall           = false;
    cells[SIZE-1][SIZE-1].isWall = false;
}
