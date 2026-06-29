// #pragma once
// #include <string>
// #include <fstream>

// // leader board ma bi min heap use hoti ha
// // es ma best survive kr jata ha or worsr remove ffrom root 
// // aik player ka complete record store krna 
// struct LBEntry
// {
//     std::string name;
//     int score=0, moves=0, coins=0;
//     LBEntry()=default;
//     LBEntry(const std::string& n,int s,int m,int c):name(n),score(s),moves(m),coins(c){}
// };

// // Min-Heap leaderboard — top 10 scores
// // Root = lowest score → O(1) eviction check, O(log N) insert
// class Leaderboard
// {
// public:
//     static const int TOP=10;
// private:
//     LBEntry heap[TOP];
//     int     sz=0;

//     void swp(LBEntry& a,LBEntry& b){ LBEntry t=a;a=b;b=t; }
//     void up(int i){ while(i>0){ int p=(i-1)/2; if(heap[p].score>heap[i].score){swp(heap[p],heap[i]);i=p;}else break; } }
//     void dn(int i){ while(true){ int s=i,l=2*i+1,r=2*i+2; if(l<sz&&heap[l].score<heap[s].score)s=l; if(r<sz&&heap[r].score<heap[s].score)s=r; if(s!=i){swp(heap[s],heap[i]);i=s;}else break; } }

//     int findName(const std::string& n) const
//     { for(int i=0;i<sz;i++) if(heap[i].name==n) return i; return -1; }

//     // max-heapify on scratch array (for sorted display only)
//     static void maxDn(LBEntry* a,int n,int i)
//     { while(true){ int lg=i,l=2*i+1,r=2*i+2; if(l<n&&a[l].score>a[lg].score)lg=l; if(r<n&&a[r].score>a[lg].score)lg=r; if(lg!=i){LBEntry t=a[i];a[i]=a[lg];a[lg]=t;i=lg;}else break; } }

// public:
//     Leaderboard(){ load(); }

//     bool qualifies(int score) const { return sz<TOP || score>heap[0].score; }

//     void add(const std::string& name, int score, int moves, int coins)
//     {
//         int slot=findName(name);
//         if(slot!=-1){ if(score>heap[slot].score){ heap[slot]={name,score,moves,coins}; up(slot); dn(slot); save(); } return; }
//         LBEntry e(name,score,moves,coins);
//         if(sz<TOP){ heap[sz]=e; up(sz++); }
//         else if(score>heap[0].score){ heap[0]=e; dn(0); }
//         save();
//     }
//     // display purpose 
//     // Returns entries sorted descending by score — O(N log N)
//     void getSorted(LBEntry out[], int& cnt) const
//     {
//         cnt=sz; if(!sz) return;
//         LBEntry sc[TOP]; for(int i=0;i<sz;i++) sc[i]=heap[i];
//         for(int i=sz/2-1;i>=0;i--) maxDn(sc,sz,i);
//         int n=sz;
//         for(int i=0;i<sz;i++){ out[i]=sc[0]; sc[0]=sc[--n]; maxDn(sc,n,0); }
//     }

//     void save() const
//     {
//         std::ofstream f("leaderboard.txt");
//         for(int i=0;i<sz;i++) f<<heap[i].score<<" "<<heap[i].moves<<" "<<heap[i].coins<<" "<<heap[i].name<<"\n";
//     }
//     void load()
//     {
//         sz=0; std::ifstream f("leaderboard.txt"); if(!f) return;
//         int s,m,c; std::string n;
//         while(f>>s>>m>>c&&std::getline(f,n)){
//             if(!n.empty()&&n[0]==' ')n=n.substr(1);
//             if(sz<TOP){ heap[sz]={n,s,m,c}; up(sz++); }
//         }
//     }
// };



#pragma once
#include <string>
#include <fstream>

// ============================================================
// LBEntry
// Ek player ka complete leaderboard record store karta hai
// ============================================================

// Leaderboard Entry structure
struct LBEntry
{
    std::string name;     // Player ka username

    int score = 0;        // Player ka final score
    int moves = 0;        // Total moves used
    int coins = 0;        // Coins collected

    // Default constructor
    LBEntry() = default;

    // Parameterized constructor
    // New entry create karne ke liye
    LBEntry(const std::string& n, int s, int m, int c)
        : name(n), score(s), moves(m), coins(c) {}
};


// ============================================================
// Leaderboard Class
// Min-Heap implementation
// Root = lowest score
// ============================================================

class Leaderboard
{
public:

    // Top sirf 10 players store karne hain
    static const int TOP = 10;

private:

    // Heap array
    // Complete Binary Tree ko array me represent kia gaya
    LBEntry heap[TOP];

    // Current heap size
    int sz = 0;


    // ========================================================
    // Swap Function
    // Do heap entries exchange karta hai
    // ========================================================

    void swp(LBEntry& a, LBEntry& b)
    {
        LBEntry t = a;
        a = b;
        b = t;
    }


    // ========================================================
    // Heapify UP
    // Insert ke baad heap property maintain karta hai
    // Child smaller ho to upar move karega
    // ========================================================

    void up(int i)
    {
        // Jab tak root tak nahi pohanchte
        while (i > 0)
        {
            // Parent index formula
            int p = (i - 1) / 2;

            // Agar parent score greater hai
            // Min-Heap violate ho rahi
            if (heap[p].score > heap[i].score)
            {
                // Parent aur child swap
                swp(heap[p], heap[i]);

                // Ab new position parent ban gayi
                i = p;
            }
            else
                break;
        }
    }


    // ========================================================
    // Heapify DOWN
    // Root replace hone ke baad heap fix karta hai
    // ========================================================

    void dn(int i)
    {
        while (true)
        {
            // Assume current node smallest hai
            int s = i;

            // Left child formula
            int l = 2 * i + 1;

            // Right child formula
            int r = 2 * i + 2;

            // Agar left child smaller hai
            if (l < sz && heap[l].score < heap[s].score)
                s = l;

            // Agar right child aur bhi smaller hai
            if (r < sz && heap[r].score < heap[s].score)
                s = r;

            // Agar smallest current node nahi
            if (s != i)
            {
                // Swap karo
                swp(heap[s], heap[i]);

                // Neeche continue karo
                i = s;
            }
            else
                break;
        }
    }


    // ========================================================
    // findName()
    // Existing player ko search karta hai
    // ========================================================

    int findName(const std::string& n) const
    {
        // Linear search
        for (int i = 0; i < sz; i++)

            // Agar name match ho gaya
            if (heap[i].name == n)
                return i;

        // Nahi mila
        return -1;
    }


    // ========================================================
    // maxDn()
    // Temporary Max-Heap banata hai
    // Sirf display sorting ke liye
    // ========================================================

    static void maxDn(LBEntry* a, int n, int i)
    {
        while (true)
        {
            // Assume current node largest hai
            int lg = i;

            // Left child
            int l = 2 * i + 1;

            // Right child
            int r = 2 * i + 2;

            // Agar left child larger hai
            if (l < n && a[l].score > a[lg].score)
                lg = l;

            // Agar right child aur bhi larger hai
            if (r < n && a[r].score > a[lg].score)
                lg = r;

            // Agar largest current node nahi
            if (lg != i)
            {
                // Swap
                LBEntry t = a[i];
                a[i] = a[lg];
                a[lg] = t;

                // Continue downward
                i = lg;
            }
            else
                break;
        }
    }

public:

    // ========================================================
    // Constructor
    // Game start hote hi leaderboard load karega
    // ========================================================

    Leaderboard()
    {
        load();
    }


    // ========================================================
    // qualifies()
    // Check karta hai new score leaderboard me a sakta hai?
    // ========================================================

    bool qualifies(int score) const
    {
        // Agar heap full nahi
        // ya score root se greater hai
        return sz < TOP || score > heap[0].score;
    }


    // ========================================================
    // add()
    // New player score insert karta hai
    // ========================================================

    void add(const std::string& name,
             int score,
             int moves,
             int coins)
    {
        // Existing player search karo
        int slot = findName(name);


        // ====================================================
        // Agar player already exist karta hai
        // ====================================================

        if (slot != -1)
        {
            // Sirf better score update karo
            if (score > heap[slot].score)
            {
                // Record replace
                heap[slot] = { name, score, moves, coins };

                // Heap property fix
                up(slot);
                dn(slot);

                // File save
                save();
            }

            return;
        }


        // New entry object
        LBEntry e(name, score, moves, coins);


        // ====================================================
        // Agar heap full nahi hai
        // ====================================================

        if (sz < TOP)
        {
            // End me insert
            heap[sz] = e;

            // Heapify up
            up(sz++);   // sz++ ke baad size increase
        }


        // ====================================================
        // Heap full hai
        // ====================================================

        else if (score > heap[0].score)
        {
            // Root replace
            // Root = lowest score
            heap[0] = e;

            // Heapify down
            dn(0);
        }

        // File me save
        save();
    }


    // ========================================================
    // getSorted()
    // Display ke liye descending order sorting
    // ========================================================

    void getSorted(LBEntry out[], int& cnt) const
    {
        cnt = sz;

        // Empty heap
        if (!sz)
            return;


        // Scratch copy
        LBEntry sc[TOP];

        // Heap copy
        for (int i = 0; i < sz; i++)
            sc[i] = heap[i];


        // ====================================================
        // Max-Heap build karo
        // ====================================================

        for (int i = sz / 2 - 1; i >= 0; i--)
            maxDn(sc, sz, i);


        // Current size
        int n = sz;


        // ====================================================
        // Heap Sort
        // ====================================================

        for (int i = 0; i < sz; i++)
        {
            // Largest value output me
            out[i] = sc[0];

            // Last node root pe lao
            sc[0] = sc[--n];

            // Heap dobara fix
            maxDn(sc, n, 0);
        }
    }


    // ========================================================
    // save()
    // Leaderboard ko file me save karta hai
    // ========================================================

    void save() const
    {
        // File open
        std::ofstream f("leaderboard.txt");

        // Sare records write karo
        for (int i = 0; i < sz; i++)
        {
            f << heap[i].score << " "
              << heap[i].moves << " "
              << heap[i].coins << " "
              << heap[i].name << "\n";
        }
    }


    // ========================================================
    // load()
    // File se leaderboard load karta hai
    // ========================================================

    void load()
    {
        // Heap reset
        sz = 0;

        // File open
        std::ifstream f("leaderboard.txt");

        // Agar file nahi mili
        if (!f)
            return;

        int s, m, c;
        std::string n;

        // File read loop
        while (f >> s >> m >> c && std::getline(f, n))
        {
            // Extra leading space remove
            if (!n.empty() && n[0] == ' ')
                n = n.substr(1);

            // Heap insert
            if (sz < TOP)
            {
                heap[sz] = { n, s, m, c };

                // Heap property maintain
                up(sz++);
            }
        }
    }
};