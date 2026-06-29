#pragma once
#include "PlayerProfile.h"
#include <string>

// Custom Hash Table — double hashing, O(1) average lookup
// Capacity 67 (prime), load capped at 70%
class HashTable
{
public:
    static const int CAP = 67;
private:
    PlayerProfile slots[CAP];
    int           count = 0;

    unsigned long djb2(const std::string& k) const
    { unsigned long h=5381; for(char c:k) h=((h<<5)+h)^(unsigned char)c; return h; }

    int h1(const std::string& k) const { return (int)(djb2(k)%CAP); }
    int h2(const std::string& k) const { return 1+(int)(djb2(k)%(CAP-1)); }

    int probe(const std::string& k) const
    {
        int s=h1(k), st=h2(k), fd=-1;
        for(int i=0;i<CAP;i++){
            int idx=(s+i*st)%CAP;
            if(!slots[idx].occupied&&!slots[idx].deleted) return fd!=-1?fd:idx;
            if(slots[idx].deleted){ if(fd==-1)fd=idx; continue; }
            if(slots[idx].username==k) return idx;
        }
        return fd;
    }
public:
    bool insert(const PlayerProfile& p)
    {
        if(count>=CAP*7/10) return false;
        int i=probe(p.username); if(i==-1) return false;
        // same usrname nhi a sakta 
        // double hasing bi with different name ka liye useful ha 
        if(slots[i].occupied&&!slots[i].deleted&&slots[i].username==p.username) return false;
        slots[i]=p; slots[i].occupied=true; slots[i].deleted=false; count++; return true;
    }
    PlayerProfile* lookup(const std::string& u)
    {
        int i=probe(u); if(i==-1) return nullptr;
        if(slots[i].occupied&&!slots[i].deleted&&slots[i].username==u) return &slots[i];
        return nullptr;
    }
    bool exists(const std::string& u){ return lookup(u)!=nullptr; }
    int getAll(PlayerProfile out[], int mx) const
    { int n=0; for(int i=0;i<CAP&&n<mx;i++) if(slots[i].occupied&&!slots[i].deleted) out[n++]=slots[i]; return n; }
};
