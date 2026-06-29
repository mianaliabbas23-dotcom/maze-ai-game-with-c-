#pragma once

// ─────────────────────────────────────────────────────────────
//  Custom Stack  (LIFO)
//  DSA: Stack — used by DFS maze generator for backtracking
// ─────────────────────────────────────────────────────────────

class Stack
{
private:
    static const int MAX = 1200;   // 31*31 = 961 — headroom added

    int arrX[MAX];
    int arrY[MAX];
    int top;

public:
    Stack() : top(-1) {}

    bool isEmpty() const { return top < 0; }
    bool isFull()  const { return top >= MAX - 1; }

    void push(int x, int y)
    {
        if (isFull()) return;
        arrX[++top] = x;
        arrY[top]   = y;
    }

    void pop()
    {
        if (isEmpty()) return;
        top--;
    }

    int peekX() const { return isEmpty() ? -1 : arrX[top]; }
    int peekY() const { return isEmpty() ? -1 : arrY[top]; }
    int size()  const { return top + 1; }
};
