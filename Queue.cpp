#include "Queue.h"

Queue::Queue()
    : frontIdx(0), rearIdx(0), count(0)
{
    for (int i = 0; i < MAX; i++) { arrX[i] = 0; arrY[i] = 0; }
}

bool Queue::isEmpty() const { return count == 0; }
bool Queue::isFull()  const { return count == MAX; }

void Queue::enqueue(int x, int y)
{
    if (isFull()) return;
    arrX[rearIdx] = x;
    arrY[rearIdx] = y;
    rearIdx = (rearIdx + 1) % MAX;
    count++;
}

void Queue::dequeue()
{
    if (isEmpty()) return;
    frontIdx = (frontIdx + 1) % MAX;
    count--;
}

int Queue::frontX() const { return isEmpty() ? -1 : arrX[frontIdx]; }
int Queue::frontY() const { return isEmpty() ? -1 : arrY[frontIdx]; }
