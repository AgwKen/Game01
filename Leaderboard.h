#pragma once
#include <cstdint>

struct RunResult; // from RunManager.h

// What we store per run
struct LeaderboardEntry
{
    char     name[16];     // "PLAYER", "AAA", etc.
    int      goals;
    int      coins;
    int      score;        // computed for sorting
    int64_t  timestamp;    // time(nullptr)
};

void Leaderboard_Initialize();     // calls Load internally
void Leaderboard_Finalize();       // calls Save internally

void Leaderboard_Clear();
void Leaderboard_Add(const char* name, const RunResult& r);

int  Leaderboard_GetCount();
const LeaderboardEntry& Leaderboard_GetEntry(int index);
int  Leaderboard_ComputeScore(const RunResult& r);
bool Leaderboard_WouldEnterTop10(const RunResult& r);