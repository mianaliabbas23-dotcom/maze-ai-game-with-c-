#pragma once

// ─────────────────────────────────────────────────────────────
//  Custom Singly Linked List
//  DSA: Linked List — AI stores and traverses its path node-by-node
// ─────────────────────────────────────────────────────────────

struct PathNode
{
    int x, y;
    PathNode* next;
    PathNode(int x, int y) : x(x), y(y), next(nullptr) {}
};

class LinkedList
{
private:
    PathNode* head;
    PathNode* tail;     // O(1) append — no more O(N) tail walk
    PathNode* current;
    int       listSize;

public:
    LinkedList() : head(nullptr), tail(nullptr), current(nullptr), listSize(0) {}
    ~LinkedList() { clear(); }

    void clear()
    {
        PathNode* tmp = head;
        while (tmp) { PathNode* n = tmp->next; delete tmp; tmp = n; }
        head = tail = current = nullptr;
        listSize = 0;
    }

    // O(1) append using tail pointer — was O(N) before
    void append(int x, int y)
    {
        PathNode* node = new PathNode(x, y);
        if (!head) { head = tail = current = node; }
        else       { tail->next = node; tail = node; }
        listSize++;
    }

    bool advance()
    {
        if (!current || !current->next) return false;
        current = current->next;
        return true;
    }

    void resetCursor() { current = head; }

    int currentX() const { return current ? current->x : -1; }
    int currentY() const { return current ? current->y : -1; }

    bool isEmpty()  const { return head == nullptr; }
    bool isAtEnd()  const { return !current || !current->next; }
    int  size()     const { return listSize; }

    PathNode* getHead() const { return head; }
};
