#include "Pathfinding.h"
#include "Queue.h"
#include "PriorityQueue.h"
#include <algorithm>

int Pathfinding::heuristic(int x,int y,int goalX,int goalY){
    int dx=x-goalX; if(dx<0)dx=-dx;
    int dy=y-goalY; if(dy<0)dy=-dy;
    return dx+dy;
}
void Pathfinding::fillLinkedList(const std::vector<std::pair<int,int>>& path,LinkedList& list){
    list.clear();
    for(auto& p:path) list.append(p.first,p.second);
    list.resetCursor();
}/// BFS algprthm
std::vector<std::pair<int,int>> Pathfinding::BFS(Grid& grid,int startX,int startY,int goalX,int goalY){
    const int SZ=Grid::SIZE;
    std::vector<std::pair<int,int>> empty;
    if(startX<0||startY<0||startX>=SZ||startY>=SZ) return empty;
    if(goalX<0||goalY<0||goalX>=SZ||goalY>=SZ) return empty;
    if(grid.getCell(startX,startY).isWall) return empty;
    if(grid.getCell(goalX,goalY).isWall) return empty;
    bool visited[Grid::SIZE][Grid::SIZE]={};
    std::pair<int,int> parent[Grid::SIZE][Grid::SIZE];
    for(int i=0;i<SZ;i++) for(int j=0;j<SZ;j++) parent[i][j]={-1,-1};
    Queue q; q.enqueue(startX,startY); visited[startX][startY]=true;
    const int dx[4]={1,-1,0,0},dy[4]={0,0,1,-1};
    bool found=false;
    while(!q.isEmpty()){
        int cx=q.frontX(),cy=q.frontY(); q.dequeue();
        if(cx==goalX&&cy==goalY){found=true;break;}
        for(int i=0;i<4;i++){
            int nx=cx+dx[i],ny=cy+dy[i];
            if(nx>=0&&ny>=0&&nx<SZ&&ny<SZ&&!visited[nx][ny]&&!grid.getCell(nx,ny).isWall){
                visited[nx][ny]=true; parent[nx][ny]={cx,cy}; q.enqueue(nx,ny);
            }
        }
    }
    // backtrack reverse path data ha kuka ya goal ko as a start calculate krta ha 

    std::vector<std::pair<int,int>> path;
    if(!found) return path;
    int x=goalX,y=goalY;
    while(!(x==startX&&y==startY)){path.push_back({x,y});auto p=parent[x][y];x=p.first;y=p.second;}
    path.push_back({startX,startY});
    std::reverse(path.begin(),path.end());
    return path;
}
// A* algorithm 
std::vector<std::pair<int,int>> Pathfinding::AStar(Grid& grid,int startX,int startY,int goalX,int goalY){
    const int SZ=Grid::SIZE;
    std::vector<std::pair<int,int>> empty;
    if(startX<0||startY<0||startX>=SZ||startY>=SZ) return empty;
    if(goalX<0||goalY<0||goalX>=SZ||goalY>=SZ) return empty;
    if(grid.getCell(startX,startY).isWall) return empty;
    if(grid.getCell(goalX,goalY).isWall) return empty;
    int gCost[Grid::SIZE][Grid::SIZE]; bool closed[Grid::SIZE][Grid::SIZE];
    std::pair<int,int> parent[Grid::SIZE][Grid::SIZE];
    for(int i=0;i<SZ;i++) for(int j=0;j<SZ;j++){gCost[i][j]=999999;closed[i][j]=false;parent[i][j]={-1,-1};}
    gCost[startX][startY]=0;
    PriorityQueue pq; pq.push(startX,startY,heuristic(startX,startY,goalX,goalY));
    const int dx[4]={1,-1,0,0},dy[4]={0,0,1,-1};
    bool found=false;
    while(!pq.isEmpty()){
        auto curr=pq.top(); pq.pop();
        int cx=curr.x,cy=curr.y;
        // duplicate node ignore krne ka liye
        // closed array use ki ha 
        if(closed[cx][cy]) continue; closed[cx][cy]=true;
        if(cx==goalX&&cy==goalY){found=true;break;}
        for(int i=0;i<4;i++){
            int nx=cx+dx[i],ny=cy+dy[i];
            if(nx<0||ny<0||nx>=SZ||ny>=SZ) continue;
            if(closed[nx][ny]||grid.getCell(nx,ny).isWall) continue;
            int newG=gCost[cx][cy]+1;
            if(newG<gCost[nx][ny]){gCost[nx][ny]=newG;parent[nx][ny]={cx,cy};pq.push(nx,ny,newG+heuristic(nx,ny,goalX,goalY));}
        }
    }
    std::vector<std::pair<int,int>> path;
    if(!found) return path;
    int x=goalX,y=goalY;
    while(!(x==startX&&y==startY)){path.push_back({x,y});auto p=parent[x][y];x=p.first;y=p.second;}
    path.push_back({startX,startY});
    std::reverse(path.begin(),path.end());
    return path;
}
