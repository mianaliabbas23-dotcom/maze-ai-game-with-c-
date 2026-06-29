#pragma once

// ─────────────────────────────────────────────────────────────
//  Custom Priority Queue (Min-Heap)
//  DSA: Heap — used by A* to always expand lowest f(n) node
// ─────────────────────────────────────────────────────────────

struct PQNode
{
    int x, y;
    int f;   // f(n) = g(n) + h(n)
};

class PriorityQueue
{
private:
    static const int MAX = 4096;
    PQNode heap[MAX];
    int    heapSize;

    void swap(PQNode& a, PQNode& b) { PQNode t = a; a = b; b = t; }

    void heapifyUp(int idx)
    {
        while (idx > 0)
        {
            int p = (idx - 1) / 2;
            if (heap[p].f > heap[idx].f) { swap(heap[p], heap[idx]); idx = p; }
            else break;
        }
    }

    void heapifyDown(int idx)
    {
        while (true)
        {
            int smallest = idx;
            int l = 2*idx+1, r = 2*idx+2;
            if (l < heapSize && heap[l].f < heap[smallest].f) smallest = l;
            if (r < heapSize && heap[r].f < heap[smallest].f) smallest = r;
            if (smallest != idx) { swap(heap[smallest], heap[idx]); idx = smallest; }
            else break;
        }
    }

public:
    PriorityQueue() : heapSize(0) {}

    bool isEmpty() const { return heapSize == 0; }
    bool isFull()  const { return heapSize >= MAX; }

    void push(int x, int y, int f)
    {
        if (isFull()) return;
        heap[heapSize] = { x, y, f };
        heapifyUp(heapSize++);
    }

    PQNode top() const { return heap[0]; }

    void pop()
    {
        if (isEmpty()) return;
        heap[0] = heap[--heapSize];
        heapifyDown(0);
    }
};
