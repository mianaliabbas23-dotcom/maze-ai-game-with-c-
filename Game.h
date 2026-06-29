#pragma once
#include <SFML/Graphics.hpp>
#include "Grid.h"
#include "Player.h"
#include "LinkedList.h"
#include "SoundManager.h"
#include "AuthManager.h"
#include "Leaderboard.h"
#include <vector>
#include <utility>// pair ko import krwanr ka lia 
// agr ikathy aye to unko pair ki form ma store krny ka lia 
#include <string>

enum class GameState { LOGIN, SIGNUP, START, PLAYING, PAUSED, VISUALIZING, WIN, LOSE, LEADERBOARD };
enum class AIMode    { PATROL, CHASE };
enum class AuthField { USERNAME, PASSWORD, CONFIRM };


// ya pathfindings algoriths kos tep by step visualize krta ha 
struct VizFrame
{
    // sd dynaimc dynaimc array ha jo 2 coordinate ko store krta ha 
    // -1,-1 ya edge case handling ha ka start ma koi node visited nhi
    std::pair<int,int>              newVisited  = {-1,-1};
    std::vector<std::pair<int,int>> newFrontier;
    std::vector<std::pair<int,int>> removedFrontier;
    std::vector<std::pair<int,int>> finalPath;
};

class Game
{
    // ── Goal / AI ─────────────────────────────────────────────
    int goalX = Grid::SIZE - 1;
    int goalY = Grid::SIZE - 1;
    int aiX   = Grid::SIZE - 1;
    int aiY   = Grid::SIZE - 1;
    AIMode aiMode     = AIMode::PATROL;
    AIMode prevAIMode = AIMode::PATROL;

    LinkedList aiPath;
    sf::Clock  pathClock;    // 1.2 sec ka bad AI move kry ga 
    int        repathCounter = 0;
    bool       newPathReady  = false;

    // track player movement history where ai mostly move 

    int heatMap[Grid::SIZE][Grid::SIZE] = {};

    // ── Coins ─────────────────────────────────────────────────
    bool coinGrid[Grid::SIZE][Grid::SIZE] = {};
    int  coinsCollected = 0;
    int  coinsTotal     = 0;

    // ── State ─────────────────────────────────────────────────
    GameState state     = GameState::LOGIN;
    GameState prevState = GameState::LOGIN;

    // ── Visualizer ────────────────────────────────────────────
   
    bool      vizUseBFS = true;
    std::vector<VizFrame> vizFrames;
    // "Sir, C++ mein agar koi function const (constant) ho,
    //  toh uske andar hum class ke variables change nahi kar sakte. 
    //  Lekin mutable keyword lagane se compiler humein ijazat 
    //  de deta hai ke is makhsoos variable ko const function ke 
    //  andar bhi modify kiya ja sake. Visualizer ko draw (render) karte waqt hum 
    //  logic change nahi karna chahte (render function const hona chahiye),
    //   lekin drawing ke liye vectors ko update karna parta hai,
    //    isliye isko mutable banaya hai."
    mutable std::vector<std::pair<int,int>> vizVisited;
    mutable std::vector<std::pair<int,int>> vizFrontier;
    mutable int vizLastReplayIdx = -1;
    int       vizFrameIdx = 0;
    sf::Clock vizClock;

    // ── Timers / misc ─────────────────────────────────────────
    sf::Clock animClock;
    sf::Clock moveSoundClock;
    sf::Clock winMusicTimer;
    sf::Clock aiRepathClock;
    bool      winMusicPending  = false;
    bool      loseMusicPending = false;

    sf::Font  font;
    bool      fontLoaded = false;

    int moveCount = 0;
    int bestScore = 0;

    SoundManager sounds;
    sf::RenderWindow window;
    Grid   grid;
    Player player;

    // ── Auth ──────────────────────────────────────────────────
    AuthManager auth;
    std::string authUser, authPw, authConfirm, authMsg;
    AuthField   activeField  = AuthField::USERNAME;
    bool        authMsgGood  = false;
    bool        showPw       = false;

    // ── Leaderboard ───────────────────────────────────────────
    Leaderboard leaderboard;

public:
    Game();
    void run();

private:
    void processEvents();
    void update();
    void render();
    void resetGame();

    // Auth helpers
    void clearAuth();
    void handleAuthText(uint32_t c);
    void tryLogin();
    void trySignUp();

    // Coins
    void spawnCoins(); // coins random place 

    // AI
    void recalculateAIPath();
    int  manhattanToPlayer() const;
    void updateAIMode();

    void buildVizFrames(bool useBFS);

    // Render screens
    void renderLogin();
    void renderSignup();
    void renderStartScreen();
    void renderGrid();
    void renderHUD();
    void renderPauseOverlay();
    void renderVisualizer();
    void renderEndScreen(bool won);
    void renderLeaderboard();

    // UI helpers
    void drawCenteredText(const std::string& s, unsigned sz, sf::Color col, float y);
    void drawPanel(float x,float y,float w,float h,sf::Color fill,sf::Color border=sf::Color::Transparent,float=0);
    void drawInputField(const std::string& label,const std::string& val,bool active,bool masked,float x,float y,float w);
    sf::FloatRect drawButton(const std::string& label,float x,float y,float w,float h,sf::Color fill,sf::Color tc);

    // 3-D-style drawing helpers (isometric projection)
    void draw3DPlayer(float cx, float cy, float cs);
    void draw3DEnemy (float cx, float cy, float cs, bool chase);
};
