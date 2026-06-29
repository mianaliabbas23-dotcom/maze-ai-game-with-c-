#pragma once
#include "Grid.h"
#include "LinkedList.h"
#include <vector>
#include <utility>
class Pathfinding {
public:
// heuristic = goal takh estimated distance find krna 
    static int heuristic(int x,int y,int goalX,int goalY);
    static std::vector<std::pair<int,int>> BFS(Grid& grid,int startX,int startY,int goalX,int goalY);
    static std::vector<std::pair<int,int>> AStar(Grid& grid,int startX,int startY,int goalX,int goalY);
    static void fillLinkedList(const std::vector<std::pair<int,int>>& path,LinkedList& list);
};
