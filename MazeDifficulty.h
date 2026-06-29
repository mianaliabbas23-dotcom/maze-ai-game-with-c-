#pragma once
#include <string>
#include <cstdlib>

// ═══════════════════════════════════════════════════════════════
//  MazeDifficulty.h
//
//  DSA: AVL Tree (Self-Balancing Binary Search Tree)
//
//  Why AVL instead of a plain BST?
//    A BST inserted in sorted order degrades to a linked list
//    (O(N) per lookup).  AVL automatically rotates after every
//    insert to keep height ≤ 1.44 log₂ N, guaranteeing
//    O(log N) for insert and search regardless of input order.
//
//  We store three difficulty ratings (Easy=20, Medium=50, Hard=80).
//  findClosest(score) returns the nearest difficulty node in O(log N).
// ═══════════════════════════════════════════════════════════════

// ── Difficulty level enum ────────────────────────────────────
enum class DifficultyLevel { EASY, MEDIUM, HARD };

// ── Parameters that change per difficulty ────────────────────
struct DiffParams
{
    DifficultyLevel level;
    std::string     label;
    int             targetScore;    // AVL tree lookup key
    int             extraShorts;    // extra wall removals (more = easier maze)
    float           aiSpeedMult;    // multiplier on AI_MOVE_DELAY (>1 = slower AI)
    int             scoreBonus;     // added to score on win
    int             coinChancePct;  // % of open cells that become coins
    int             chaseThreshold; // Manhattan dist at which AI uses A*
};

// Returns preset parameters for each difficulty
inline DiffParams getDiffParams(DifficultyLevel lv)
{
    switch (lv)
    {
    case DifficultyLevel::EASY:
        //   Slow AI, lots of shortcuts, many coins, AI chases from far
        return { lv, "Easy",   20, 30, 1.80f, 300, 40,  9 };
    case DifficultyLevel::HARD:
        //   Fast AI, tight maze, few coins, very dangerous
        return { lv, "Hard",   80,  4, 0.65f, 800, 15,  3 };
    default: // MEDIUM
        return { lv, "Medium", 50, 12, 1.10f, 500, 28,  6 };
    }
}

// ═══════════════════════════════════════════════════════════════
//  AVL Tree Node
// ═══════════════════════════════════════════════════════════════
struct DiffNode
{
    int         diffScore;      // key used for BST ordering
    std::string label;          // "Easy", "Medium", "Hard"
    int         extraShortcuts;
    int         deadEnds;
    int         solveLen;
    int         height;
    DiffNode*   left;
    DiffNode*   right;

    DiffNode(int sc, const std::string& lbl, int ex, int de, int sl)
        : diffScore(sc), label(lbl), extraShortcuts(ex),
          deadEnds(de), solveLen(sl),
          height(1), left(nullptr), right(nullptr) {}
};

// ═══════════════════════════════════════════════════════════════
//  AVL Tree
// ═══════════════════════════════════════════════════════════════
class MazeDifficultyTree
{
private:
    DiffNode* root;

    int height(DiffNode* n) const { return n ? n->height : 0; }
    int bf(DiffNode* n)     const { return n ? height(n->left) - height(n->right) : 0; }

    void updateHeight(DiffNode* n)
    {
        if (!n) return;
        int lh = height(n->left), rh = height(n->right);
        n->height = 1 + (lh > rh ? lh : rh);
    }

    // ── Rotations ─────────────────────────────────────────────
    DiffNode* rotRight(DiffNode* y)
    {
        DiffNode* x  = y->left;
        DiffNode* T2 = x->right;
        x->right = y;  y->left = T2;
        updateHeight(y); updateHeight(x);
        return x;
    }

    DiffNode* rotLeft(DiffNode* x)
    {
        DiffNode* y  = x->right;
        DiffNode* T2 = y->left;
        y->left = x;  x->right = T2;
        updateHeight(x); updateHeight(y);
        return y;
    }

    // Rebalance after insert
    DiffNode* balance(DiffNode* n)
    {
        updateHeight(n);
        int b = bf(n);

        if (b > 1)   // Left heavy
        {
            if (bf(n->left) < 0) n->left = rotLeft(n->left);  // LR case
            return rotRight(n);
        }
        if (b < -1)  // Right heavy
        {
            if (bf(n->right) > 0) n->right = rotRight(n->right);  // RL case
            return rotLeft(n);
        }
        return n;
    }

    DiffNode* insert(DiffNode* node, int sc, const std::string& lbl, int ex, int de, int sl)
    {
        if (!node) return new DiffNode(sc, lbl, ex, de, sl);
        if      (sc < node->diffScore) node->left  = insert(node->left,  sc, lbl, ex, de, sl);
        else if (sc > node->diffScore) node->right = insert(node->right, sc, lbl, ex, de, sl);
        else { node->label = lbl; node->extraShortcuts = ex; return node; }
        return balance(node);
    }

    // O(log N) search for closest score
    DiffNode* findClosest(DiffNode* node, int target, DiffNode* best) const
    {
        if (!node) return best;
        if (!best || abs(node->diffScore - target) < abs(best->diffScore - target))
            best = node;
        if (target < node->diffScore) return findClosest(node->left,  target, best);
        else                          return findClosest(node->right, target, best);
    }

    void destroy(DiffNode* n)
    {
        if (!n) return;
        destroy(n->left); destroy(n->right);
        delete n;
    }

public:
    MazeDifficultyTree() : root(nullptr) {}
    ~MazeDifficultyTree() { destroy(root); }

    void insert(int score, const std::string& label, int extraShortcuts, int deadEnds, int solveLen)
    { root = insert(root, score, label, extraShortcuts, deadEnds, solveLen); }

    const DiffNode* findClosest(int target) const
    { return findClosest(root, target, nullptr); }
};
