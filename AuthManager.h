#pragma once
#include "HashTable.h"
#include <string>
#include <fstream>
#include <sstream> // string ko parse krne ka liye 

enum class AuthResult { OK, TAKEN, EMPTY, TOO_LONG, SHORT_PW, NOT_FOUND, WRONG_PW };

class AuthManager
{
    HashTable      table;
    PlayerProfile* cur = nullptr;
    std::string    curName;

    static unsigned int hashPw(const std::string& pw)
    { unsigned long h=5381; for(char c:pw) h=((h<<5)+h)^(unsigned char)c; return (unsigned int)h; }

public:
    AuthManager() { load(); }

    // signup function

    AuthResult signUp(const std::string& u, const std::string& pw)
    {
        if(u.empty())          return AuthResult::EMPTY;
        if(u.size()>16)        return AuthResult::TOO_LONG;
        if(pw.size()<4)        return AuthResult::SHORT_PW;
        if(table.exists(u))    return AuthResult::TAKEN;
        PlayerProfile p(u, hashPw(pw));
        table.insert(p);  save();
        cur=table.lookup(u); curName=u;
        return AuthResult::OK;
    }

    // login function

    AuthResult login(const std::string& u, const std::string& pw)
    {
        // hashtable me search krta ha username 
        PlayerProfile* p=table.lookup(u);
        if(!p)                          return AuthResult::NOT_FOUND;
        if(p->passwordHash!=hashPw(pw)) return AuthResult::WRONG_PW;
        cur=p; curName=u; return AuthResult::OK;
    }

    void logout(){ cur=nullptr; curName=""; }

    bool               loggedIn()    const { return cur!=nullptr; }
    const std::string& username()    const { return curName; }

    void recordWin(int score)
    {
        if(!cur) return;
        cur->gamesPlayed++; cur->gamesWon++;
        if(score>cur->bestScore) cur->bestScore=score;
        save();
    }
    void recordLoss()
    { if(!cur){ return; } cur->gamesPlayed++; save(); }

    static std::string msg(AuthResult r)
    {
        switch(r){
        case AuthResult::TAKEN:    return "Username already taken";
        case AuthResult::EMPTY:    return "Username cannot be empty";
        case AuthResult::TOO_LONG: return "Username max 16 characters";
        case AuthResult::SHORT_PW: return "Password needs 4+ characters";
        case AuthResult::NOT_FOUND:return "Username not found";
        case AuthResult::WRONG_PW: return "Incorrect password";
        default:                   return "OK";
        }
    }

    void save() const
    {
        std::ofstream f("players.txt");
        // n = hashtable sy user extract krna 
        PlayerProfile all[HashTable::CAP]; int n=table.getAll(all,HashTable::CAP);
        for(int i=0;i<n;i++)
            f<<all[i].username<<" "<<all[i].passwordHash<<" "
             <<all[i].gamesPlayed<<" "<<all[i].gamesWon<<" "<<all[i].bestScore<<"\n";
    }
    void load()
    {
        std::ifstream f("players.txt"); if(!f) return;
        std::string line;
        while(std::getline(f,line)){
            if(line.empty()) continue;
            std::istringstream ss(line); PlayerProfile p; p.occupied=true;
            if(ss>>p.username>>p.passwordHash>>p.gamesPlayed>>p.gamesWon>>p.bestScore)
                table.insert(p);
        }
    }
};
