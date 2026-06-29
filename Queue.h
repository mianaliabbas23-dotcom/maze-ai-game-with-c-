#pragma once

// ─────────────────────────────────────────────────────────────
//  Custom Queue  (FIFO circular buffer)
//  DSA: Queue — used by BFS pathfinding
// ─────────────────────────────────────────────────────────────

class Queue
{
private:
    static const int MAX = 1200;  // 31*31 = 961

    int arrX[MAX];
    int arrY[MAX];
    int frontIdx;
    int rearIdx;
    int count;

public:
    Queue();

    bool isEmpty() const;
    bool isFull()  const;

    void enqueue(int x, int y);
    void dequeue();

    int frontX() const;
    int frontY() const;
};
