#include "Leaderboard.h"
#include "RunManager.h"   // for RunResult
#include <vector>
#include <algorithm>
#include <fstream>
#include <cstring>
#include <ctime>
#include <direct.h> // _mkdir

// ============================================================
// SETTINGS
// ============================================================
static constexpr int LEADERBOARD_MAX = 10;
static const char* LEADERBOARD_FILE = "Save/leaderboard.dat";

// ============================================================
// DATA
// ============================================================
static std::vector<LeaderboardEntry> g_List;

// ============================================================
// INTERNAL HELPERS
// ============================================================
static int ComputeScore(const RunResult& r)
{
    int goalPts = 500;
    float mult = 1.0f + (r.coins * 0.05f);   // 20 coins = +1.0x
    if (mult > 3.0f) mult = 3.0f;            // cap 3x
    return (int)(r.goals * goalPts * mult);
}

static void SortAndTrim()
{
    std::sort(g_List.begin(), g_List.end(),
        [](const LeaderboardEntry& a, const LeaderboardEntry& b)
        {
            if (a.score != b.score) return a.score > b.score;
            // tie-break: more goals
            if (a.goals != b.goals) return a.goals > b.goals;
            // tie-break: more coins
            if (a.coins != b.coins) return a.coins > b.coins;
            // tie-break: newer first
            return a.timestamp > b.timestamp;
        });

    if ((int)g_List.size() > LEADERBOARD_MAX)
        g_List.resize(LEADERBOARD_MAX);
}

static void SaveToFile()
{
    (void)_mkdir("Save"); // create folder if missing (safe if already exists)

    std::ofstream out(LEADERBOARD_FILE, std::ios::binary);
    if (!out.is_open()) return;

    int count = (int)g_List.size();
    out.write((const char*)&count, sizeof(count));
    if (count > 0)
        out.write((const char*)g_List.data(), sizeof(LeaderboardEntry) * count);
}

static void LoadFromFile()
{
    g_List.clear();

    std::ifstream in(LEADERBOARD_FILE, std::ios::binary);
    if (!in.is_open()) return;

    int count = 0;
    in.read((char*)&count, sizeof(count));
    if (!in.good()) return;

    if (count < 0) return;
    if (count > 1000) return; // corruption guard

    g_List.resize(count);
    if (count > 0)
        in.read((char*)g_List.data(), sizeof(LeaderboardEntry) * count);

    // In case file contains old data unsorted
    SortAndTrim();
}

// ============================================================
// PUBLIC API
// ============================================================
void Leaderboard_Initialize()
{
    LoadFromFile();
    SaveToFile(); // force create leaderboard.dat immediately
}

void Leaderboard_Finalize()
{
    SaveToFile();
    g_List.clear();
}

void Leaderboard_Clear()
{
    g_List.clear();
    SaveToFile();
}

void Leaderboard_Add(const char* name, const RunResult& r)
{
    LeaderboardEntry e{};
    e.goals = r.goals;
    e.coins = r.coins;
    e.score = ComputeScore(r);
    e.timestamp = (int64_t)time(nullptr);

    // Safe copy name into fixed buffer
    std::memset(e.name, 0, sizeof(e.name));
    if (name && name[0] != '\0')
    {
        strncpy_s(e.name, sizeof(e.name), name, _TRUNCATE);
    }
    else
    {
        strncpy_s(e.name, sizeof(e.name), "PLAYER", _TRUNCATE);
    }

    g_List.push_back(e);
    SortAndTrim();
    SaveToFile(); // save immediately so it persists even if crash
}

int Leaderboard_GetCount()
{
    return (int)g_List.size();
}

const LeaderboardEntry& Leaderboard_GetEntry(int index)
{
    // minimal safety: clamp
    if (g_List.empty())
    {
        static LeaderboardEntry dummy{};
        return dummy;
    }

    if (index < 0) index = 0;
    if (index >= (int)g_List.size()) index = (int)g_List.size() - 1;
    return g_List[index];
}

int Leaderboard_ComputeScore(const RunResult& r)
{
    return ComputeScore(r);
}

// SAME rules as SortAndTrim compare
static bool IsBetterEntry(const LeaderboardEntry& a, const LeaderboardEntry& b)
{
    if (a.score != b.score) return a.score > b.score;
    if (a.goals != b.goals) return a.goals > b.goals;
    if (a.coins != b.coins) return a.coins > b.coins;
    return a.timestamp > b.timestamp;
}

bool Leaderboard_WouldEnterTop10(const RunResult& r)
{
    LeaderboardEntry e{};
    e.goals = r.goals;
    e.coins = r.coins;
    e.score = ComputeScore(r);
    e.timestamp = (int64_t)time(nullptr);

    // If not full, it qualifies
    if ((int)g_List.size() < LEADERBOARD_MAX)
        return true;

    SortAndTrim();  
    const LeaderboardEntry& worst = g_List.back();

    return IsBetterEntry(e, worst);
}