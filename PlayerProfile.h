#pragma once
#include <string>

struct PlayerProfile
{
    std::string  username;
    unsigned int passwordHash = 0;
    int          gamesPlayed  = 0;
    int          gamesWon     = 0;
    int          bestScore    = 0;
    bool         occupied     = false;
    bool         deleted      = false;

    PlayerProfile() = default;
    PlayerProfile(const std::string& u, unsigned int ph)
        : username(u), passwordHash(ph), occupied(true) {}
};
