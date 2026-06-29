#include "Game.h"
#include "Config.h"
#include "Pathfinding.h"
#include "Queue.h"
#include "PriorityQueue.h"
#include <cstdlib>
#include <cstdint>
#include <cmath>
#include <algorithm>            
#include <string>   

// ─────────────────────────────────────────────────────────────
//  Coin settings
// ─────────────────────────────────────────────────────────────
static const int COIN_CHANCE = 25;    // % of open cells that get a coin
static const int COIN_PTS = 15;    // points per coin
static const float MY_AI_DELAY = 0.50f; // seconds per AI step (slower = fairer)

// ═══════════════════════════════════════════════════════════
//  Constructor
// ═══════════════════════════════════════════════════════════
Game::Game()
    : window(sf::VideoMode(sf::Vector2u(WIN_W, WIN_H)), "TRAPSCAPE |  DSA Project"),
    player(0, 0)
{
    window.setFramerateLimit(60);
    for (int i = 0;i < Grid::SIZE;i++) for (int j = 0;j < Grid::SIZE;j++) heatMap[i][j] = 0;

    fontLoaded = font.openFromFile("arial.ttf");
    if (!fontLoaded) fontLoaded = font.openFromFile("C:/Windows/Fonts/arial.ttf");
    if (!fontLoaded) fontLoaded = font.openFromFile("C:/Windows/Fonts/calibri.ttf");
    if (!fontLoaded) fontLoaded = font.openFromFile("C:/Windows/Fonts/segoeui.ttf");

    animClock.restart();
    sounds.stopAllMusic();
    sounds.play(SoundManager::START_MUSIC);

    clearAuth();
    state = GameState::LOGIN;
}

// ═══════════════════════════════════════════════════════════
//  Main loop
// ═══════════════════════════════════════════════════════════
void Game::run()
{
    while (window.isOpen()) { processEvents(); update(); render(); }
}

// ═══════════════════════════════════════════════════════════
//  Reset
// ═══════════════════════════════════════════════════════════
void Game::resetGame()
{
    grid = Grid(); player = Player(0, 0);
    aiX = Grid::SIZE - 1; aiY = Grid::SIZE - 1;
    aiMode = AIMode::PATROL; repathCounter = 0; newPathReady = false; moveCount = 0;
    coinsCollected = 0; coinsTotal = 0;
    for (int i = 0;i < Grid::SIZE;i++) for (int j = 0;j < Grid::SIZE;j++) { heatMap[i][j] = 0; coinGrid[i][j] = false; }
    sounds.stopAllMusic();
    moveSoundClock.restart(); aiRepathClock.restart();
    winMusicPending = false; loseMusicPending = false;
    spawnCoins();
    state = GameState::PLAYING;
    recalculateAIPath();
    animClock.restart();
}

// ═══════════════════════════════════════════════════════════
//  Coins — scatter on 25% of open cells
// ═══════════════════════════════════════════════════════════
void Game::spawnCoins()
{
    coinsCollected = 0; coinsTotal = 0;
    for (int r = 0;r < Grid::SIZE;r++)
        for (int c = 0;c < Grid::SIZE;c++)
        {
            coinGrid[r][c] = false;
            if (grid.getCell(c, r).isWall) continue;
            if (c == 0 && r == 0) continue;
            if (c == goalX && r == goalY) continue;
            if (rand() % 100 < COIN_CHANCE) { coinGrid[r][c] = true; coinsTotal++; }
        }
}

// ═══════════════════════════════════════════════════════════
//  Auth helpers
// ═══════════════════════════════════════════════════════════
void Game::clearAuth()
{
    authUser = authPw = authConfirm = authMsg = "";
    activeField = AuthField::USERNAME; authMsgGood = false; showPw = false;
}

void Game::handleAuthText(uint32_t c)
{
    std::string& f = (activeField == AuthField::USERNAME) ? authUser
        : (activeField == AuthField::CONFIRM) ? authConfirm : authPw;
    int mx = (activeField == AuthField::USERNAME) ? 16 : 32;
    if (c == 8) { if (!f.empty()) f.pop_back(); }
    else if (c >= 32 && c < 127 && (int)f.size() < mx) f += (char)c;
    authMsg = "";
}

void Game::tryLogin()
{
    AuthResult r = auth.login(authUser, authPw);
    if (r == AuthResult::OK) { clearAuth(); state = GameState::START; sounds.play(SoundManager::START_MUSIC); }
    else { authMsg = AuthManager::msg(r); authMsgGood = false; }
}

void Game::trySignUp()
{
    if (authPw != authConfirm) { authMsg = "Passwords do not match"; authMsgGood = false; return; }
    AuthResult r = auth.signUp(authUser, authPw);
    if (r == AuthResult::OK) { clearAuth(); state = GameState::START; sounds.play(SoundManager::START_MUSIC); }
    else { authMsg = AuthManager::msg(r); authMsgGood = false; }
}

// ═══════════════════════════════════════════════════════════
//  Input
// ═══════════════════════════════════════════════════════════
void Game::processEvents()
{
    while (auto event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>()) { window.close(); return; }

        // ── Text input for auth fields ────────────────────────
        if (auto* t = event->getIf<sf::Event::TextEntered>())
        {
            if (t->unicode == 13 || t->unicode == 27 || t->unicode == 9) continue;
            if (state == GameState::LOGIN || state == GameState::SIGNUP) handleAuthText(t->unicode);
            continue;
        }

        // ── Mouse clicks ──────────────────────────────────────
        if (auto* cl = event->getIf<sf::Event::MouseButtonReleased>())
        {
            if (cl->button != sf::Mouse::Button::Left) continue;
            float mx = (float)cl->position.x, my = (float)cl->position.y;

            if (state == GameState::LOGIN)
            {
                float pw = 440, ph = 360, px = (WIN_W - pw) / 2.f, py = (WIN_H - ph) / 2.f, fx = px + 32, fw = pw - 64;
                if (mx >= fx && mx <= fx + fw && my >= py + 114 && my <= py + 152) activeField = AuthField::USERNAME;
                else if (mx >= fx && mx <= fx + fw && my >= py + 184 && my <= py + 222) activeField = AuthField::PASSWORD;
                else if (mx >= px + 32 && mx <= px + pw / 2 - 8 && my >= py + 245 && my <= py + 285) tryLogin();
                else if (mx >= px + pw / 2 + 8 && mx <= px + pw - 32 && my >= py + 245 && my <= py + 285) { clearAuth(); state = GameState::SIGNUP; }
            }
            else if (state == GameState::SIGNUP)
            {
                float pw = 440, ph = 440, px = (WIN_W - pw) / 2.f, py = (WIN_H - ph) / 2.f, fx = px + 32, fw = pw - 64;
                if (mx >= fx && mx <= fx + fw && my >= py + 110 && my <= py + 148) activeField = AuthField::USERNAME;
                else if (mx >= fx && mx <= fx + fw && my >= py + 180 && my <= py + 218) activeField = AuthField::PASSWORD;
                else if (mx >= fx && mx <= fx + fw && my >= py + 250 && my <= py + 288) activeField = AuthField::CONFIRM;
                else if (mx >= px + 32 && mx <= px + pw / 2 - 8 && my >= py + 305 && my <= py + 345) trySignUp();
                else if (mx >= px + pw / 2 + 8 && mx <= px + pw - 32 && my >= py + 305 && my <= py + 345) { clearAuth(); state = GameState::LOGIN; }
            }
            else if (state == GameState::LEADERBOARD)
            {
                float panW = WIN_W * 0.82f, panH = WIN_H * 0.88f;
                float panX = (WIN_W - panW) / 2.f, panY = (WIN_H - panH) / 2.f;
                float bw = 200, bh = 32, bx = (WIN_W - bw) / 2.f, by = panY + panH - 44;
                if (mx >= bx && mx <= bx + bw && my >= by && my <= by + bh)
                    state = (auth.loggedIn() ? GameState::START : GameState::LOGIN);
            }
            else if (state == GameState::START)
            {
                // Buttons must match renderStartScreen() exactly
                float panW = WIN_W * 0.80f, panH = WIN_H * 0.90f;
                float panX = (WIN_W - panW) / 2.f, panY = (WIN_H - panH) / 2.f;
                float btnH = 40.f, btnW = panW * 0.26f;
                float totalBtnW = btnW * 3 + 24.f;
                float btnStartX = panX + (panW - totalBtnW) / 2.f;
                float btnY = panY + panH - 62.f;

                // START button
                if (mx >= btnStartX && mx <= btnStartX + btnW && my >= btnY && my <= btnY + btnH)
                    resetGame();
                // LEADERBOARD button
                else if (mx >= btnStartX + btnW + 12.f && mx <= btnStartX + btnW * 2 + 12.f && my >= btnY && my <= btnY + btnH)
                    state = GameState::LEADERBOARD;
                // LOGOUT button
                else if (mx >= btnStartX + btnW * 2 + 24.f && mx <= btnStartX + btnW * 3 + 24.f && my >= btnY && my <= btnY + btnH)
                {
                    auth.logout(); clearAuth(); state = GameState::LOGIN;
                }
            }
            else if (state == GameState::PAUSED)
            {
                // Button coords must match renderPauseOverlay() exactly
                float pw = 360, ph = 240, px = (WIN_W - pw) / 2.f, py = (WIN_H - ph) / 2.f;
                float bw = pw * 0.65f, bh = 42, bx = px + (pw - bw) / 2.f;
                // Resume
                if (mx >= bx && mx <= bx + bw && my >= py + 56 && my <= py + 56 + bh)
                    state = GameState::PLAYING;
                // Logout
                else if (mx >= bx && mx <= bx + bw && my >= py + 108 && my <= py + 108 + bh)
                {
                    auth.logout(); clearAuth(); state = GameState::LOGIN;
                }
                // Leaderboard
                else if (mx >= bx && mx <= bx + bw && my >= py + 160 && my <= py + 160 + bh)
                    state = GameState::LEADERBOARD;
            }
            else if (state == GameState::WIN || state == GameState::LOSE)
            {
                float panW = WIN_W * 0.62f, panH = WIN_H * 0.56f;
                float panX = (WIN_W - panW) / 2.f, panY = (WIN_H - panH) / 2.f;
                float bw = panW * 0.36f, bh = 40, gap = (panW - 2 * bw) / 3.f, by = panY + panH - 56;
                if (mx >= panX + gap && mx <= panX + gap + bw && my >= by && my <= by + bh) resetGame();
                else if (mx >= panX + gap * 2 + bw && mx <= panX + gap * 2 + bw * 2 && my >= by && my <= by + bh) state = GameState::LEADERBOARD;
            }
        }

        // ── Keyboard ──────────────────────────────────────────
        if (auto* key = event->getIf<sf::Event::KeyPressed>())
        {
            if (state == GameState::LOGIN)
            {
                if (key->code == sf::Keyboard::Key::Tab) activeField = (activeField == AuthField::USERNAME) ? AuthField::PASSWORD : AuthField::USERNAME;
                else if (key->code == sf::Keyboard::Key::Enter) tryLogin();
                else if (key->code == sf::Keyboard::Key::F2) showPw = !showPw;
                continue;
            }
            if (state == GameState::SIGNUP)
            {
                if (key->code == sf::Keyboard::Key::Tab) {
                    if (activeField == AuthField::USERNAME) activeField = AuthField::PASSWORD;
                    else if (activeField == AuthField::PASSWORD) activeField = AuthField::CONFIRM;
                    else activeField = AuthField::USERNAME;
                }
                else if (key->code == sf::Keyboard::Key::Enter) trySignUp();
                else if (key->code == sf::Keyboard::Key::Escape) { clearAuth(); state = GameState::LOGIN; }
                else if (key->code == sf::Keyboard::Key::F2) showPw = !showPw;
                continue;
            }
            if (state == GameState::START)
            {
                if (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space) resetGame();
                else if (key->code == sf::Keyboard::Key::L) state = GameState::LEADERBOARD;
                return;
            }
            if (state == GameState::LEADERBOARD)
            {
                if (key->code == sf::Keyboard::Key::Escape || key->code == sf::Keyboard::Key::Enter)
                    state = (auth.loggedIn() ? GameState::START : GameState::LOGIN);
                return;
            }
            if (state == GameState::WIN || state == GameState::LOSE)
            {
                if (key->code == sf::Keyboard::Key::R || key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space) resetGame();
                else if (key->code == sf::Keyboard::Key::L) state = GameState::LEADERBOARD;
                return;
            }
            if (state == GameState::PAUSED)
            {
                if (key->code == sf::Keyboard::Key::P || key->code == sf::Keyboard::Key::Escape)
                    state = GameState::PLAYING;
                continue;
            }
            if (key->code == sf::Keyboard::Key::V && state == GameState::PLAYING)
            {
                state = GameState::VISUALIZING; vizUseBFS = true;  buildVizFrames(true);  vizFrameIdx = 0; vizClock.restart(); return;
            }
            if (key->code == sf::Keyboard::Key::A && state == GameState::PLAYING)
            {
                state = GameState::VISUALIZING; vizUseBFS = false; buildVizFrames(false); vizFrameIdx = 0; vizClock.restart(); return;
            }
            if (key->code == sf::Keyboard::Key::Escape)
            {
                if (state == GameState::VISUALIZING) state = GameState::PLAYING; return;
            }
            if (key->code == sf::Keyboard::Key::R) { resetGame(); return; }
            if (key->code == sf::Keyboard::Key::L && state == GameState::PLAYING) { state = GameState::LEADERBOARD; return; }
            if (key->code == sf::Keyboard::Key::P && state == GameState::PLAYING) { state = GameState::PAUSED; return; }
            if (state != GameState::PLAYING) return;

            bool moved = false, tried = false;
            if (key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W) { moved = player.move(0, -1, grid); tried = true; }
            else if (key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S) { moved = player.move(0, 1, grid); tried = true; }
            else if (key->code == sf::Keyboard::Key::Left || key->code == sf::Keyboard::Key::Q) { moved = player.move(-1, 0, grid); tried = true; }
            else if (key->code == sf::Keyboard::Key::Right || key->code == sf::Keyboard::Key::D) { moved = player.move(1, 0, grid); tried = true; }

            if (tried && !moved) sounds.play(SoundManager::WALL_HIT);
            if (moved)
            {
                if (moveSoundClock.getElapsedTime().asMilliseconds() > 80) { sounds.play(SoundManager::MOVE); moveSoundClock.restart(); }
                heatMap[player.getX()][player.getY()]++;
                player.addScore(SCORE_PER_MOVE);
                moveCount++;

                // Collect coin
                int px = player.getX(), py = player.getY();
                if (coinGrid[py][px]) { coinGrid[py][px] = false; coinsCollected++; player.addScore(COIN_PTS); }

                repathCounter++;
                if (repathCounter >= AI_REPATH_INTERVAL) { newPathReady = true; repathCounter = 0; }
            }
        }
    }
}

// ═══════════════════════════════════════════════════════════
//  Update
// ═══════════════════════════════════════════════════════════
void Game::update()
{
    if (state == GameState::VISUALIZING)
    {
        if (vizClock.getElapsedTime().asSeconds() > VIZ_STEP_DELAY)
        {
            if (vizFrameIdx < (int)vizFrames.size() - 1) { vizFrameIdx++; sounds.play(SoundManager::VIZ_TICK); } vizClock.restart();
        }
        return;
    }
    if (state != GameState::PLAYING) return;

    player.update();
    prevAIMode = aiMode; updateAIMode();
    if (aiMode == AIMode::CHASE && prevAIMode == AIMode::PATROL) sounds.play(SoundManager::CHASE_ALERT);

    bool pathExhausted = aiPath.isEmpty() || aiPath.isAtEnd();
    bool timedOut = aiRepathClock.getElapsedTime().asSeconds() > 1.2f;
    if (newPathReady || pathExhausted || timedOut) { recalculateAIPath(); newPathReady = false; aiRepathClock.restart(); }

    if (!aiPath.isEmpty() && !aiPath.isAtEnd())
    {
        if (pathClock.getElapsedTime().asSeconds() > MY_AI_DELAY)
        {
            aiPath.advance();
            int nx = aiPath.currentX(), ny = aiPath.currentY();
            if (nx >= 0 && ny >= 0 && nx < Grid::SIZE && ny < Grid::SIZE && !grid.getCell(nx, ny).isWall) { aiX = nx; aiY = ny; sounds.play(SoundManager::AI_STEP); }
            else recalculateAIPath();
            pathClock.restart();
        }
    }

    // Win
    if (player.getX() == goalX && player.getY() == goalY)
    {
        player.addScore(SCORE_WIN_BONUS);
        if (player.getScore() > bestScore) bestScore = player.getScore();
        if (leaderboard.qualifies(player.getScore()))
        {
            std::string name = auth.loggedIn() ? auth.username() : "Guest";
            leaderboard.add(name, player.getScore(), moveCount, coinsCollected);
        }
        auth.recordWin(player.getScore());
        state = GameState::WIN; sounds.play(SoundManager::WIN); winMusicTimer.restart(); winMusicPending = true;
    }
    // Lose
    if (aiX == player.getX() && aiY == player.getY())
    {
        auth.recordLoss(); state = GameState::LOSE; sounds.play(SoundManager::LOSE); winMusicTimer.restart(); loseMusicPending = true;
    }

    if (winMusicPending && winMusicTimer.getElapsedTime().asSeconds() > 1.5f) { sounds.play(SoundManager::WIN_MUSIC);  winMusicPending = false; }
    if (loseMusicPending && winMusicTimer.getElapsedTime().asSeconds() > 1.5f) { sounds.play(SoundManager::LOSE_MUSIC); loseMusicPending = false; }
}

// ═══════════════════════════════════════════════════════════
//  AI helpers
// ═══════════════════════════════════════════════════════════
int Game::manhattanToPlayer() const
{
    int dx = aiX - player.getX(); if (dx < 0)dx = -dx; int dy = aiY - player.getY(); if (dy < 0)dy = -dy; return dx + dy;
}

void Game::updateAIMode()
{
    aiMode = (manhattanToPlayer() <= CHASE_THRESHOLD) ? AIMode::CHASE : AIMode::PATROL;
}

void Game::recalculateAIPath()
{
    if (aiX < 0)aiX = 0; if (aiY < 0)aiY = 0;
    if (aiX >= Grid::SIZE)aiX = Grid::SIZE - 1; if (aiY >= Grid::SIZE)aiY = Grid::SIZE - 1;
    if (grid.getCell(aiX, aiY).isWall)
    {
        int dx[] = { 1,-1,0,0 }, dy[] = { 0,0,1,-1 }; for (int d = 0;d < 4;d++) { int nx = aiX + dx[d], ny = aiY + dy[d]; if (nx >= 0 && ny >= 0 && nx < Grid::SIZE && ny < Grid::SIZE && !grid.getCell(nx, ny).isWall) { aiX = nx;aiY = ny;break; } }
    }
    std::vector<std::pair<int, int>> raw = Pathfinding::AStar(grid, aiX, aiY, player.getX(), player.getY());
    if (raw.empty()) raw = Pathfinding::BFS(grid, aiX, aiY, player.getX(), player.getY());
    Pathfinding::fillLinkedList(raw, aiPath);
    if (!aiPath.isEmpty() && aiPath.size() > 1) aiPath.advance();
}

// ═══════════════════════════════════════════════════════════
//  Visualizer frame builder (unchanged from v2)
// ═══════════════════════════════════════════════════════════
void Game::buildVizFrames(bool useBFS)
{
    vizFrames.clear(); vizLastReplayIdx = -1; vizVisited.clear(); vizFrontier.clear();
    const int SZ = Grid::SIZE; int startX = 0, startY = 0, gX = goalX, gY = goalY;
    bool visited[31][31] = {}, inFrontier[31][31] = {};
    std::pair<int, int> parent[31][31]; for (int i = 0;i < SZ;i++) for (int j = 0;j < SZ;j++) parent[i][j] = { -1,-1 };
    const int dx[4] = { 1,-1,0,0 }, dy[4] = { 0,0,1,-1 };
    auto emit = [&](std::pair<int, int> nv, std::vector<std::pair<int, int>> af, std::vector<std::pair<int, int>> rf)
        { VizFrame f; f.newVisited = nv; f.newFrontier = std::move(af); f.removedFrontier = std::move(rf); vizFrames.push_back(std::move(f)); };

    if (useBFS)
    {
        Queue q; q.enqueue(startX, startY); visited[startX][startY] = inFrontier[startX][startY] = true;
        emit({ -1,-1 }, { {startX,startY} }, {}); bool found = false;
        while (!q.isEmpty()) {
            int cx = q.frontX(), cy = q.frontY(); q.dequeue(); inFrontier[cx][cy] = false;
            std::vector<std::pair<int, int>> added;
            for (int i = 0;i < 4;i++) { int nx = cx + dx[i], ny = cy + dy[i]; if (nx < 0 || ny < 0 || nx >= SZ || ny >= SZ) continue; if (visited[nx][ny] || grid.getCell(nx, ny).isWall) continue; visited[nx][ny] = inFrontier[nx][ny] = true; parent[nx][ny] = { cx,cy }; q.enqueue(nx, ny); added.push_back({ nx,ny }); }
            emit({ cx,cy }, std::move(added), { {cx,cy} }); if (cx == gX && cy == gY) { found = true;break; }
        }
        if (found) { std::vector<std::pair<int, int>> fp; int x = gX, y = gY; while (!(x == startX && y == startY)) { fp.push_back({ x,y });auto p = parent[x][y];if (p.first == -1)break;x = p.first;y = p.second; } fp.push_back({ startX,startY }); std::reverse(fp.begin(), fp.end()); VizFrame last; last.finalPath = std::move(fp); vizFrames.push_back(std::move(last)); }
    }
    else
    {
        int gCost[31][31]; bool closed[31][31];
        for (int i = 0;i < SZ;i++) for (int j = 0;j < SZ;j++) { gCost[i][j] = 999999;closed[i][j] = false; }
        gCost[startX][startY] = 0; PriorityQueue pq; pq.push(startX, startY, Pathfinding::heuristic(startX, startY, gX, gY));
        visited[startX][startY] = inFrontier[startX][startY] = true; emit({ -1,-1 }, { {startX,startY} }, {}); bool found = false;
        while (!pq.isEmpty()) {
            auto curr = pq.top(); pq.pop(); int cx = curr.x, cy = curr.y;
            if (closed[cx][cy]) continue; closed[cx][cy] = true; inFrontier[cx][cy] = false;
            std::vector<std::pair<int, int>> added;
            for (int i = 0;i < 4;i++) { int nx = cx + dx[i], ny = cy + dy[i]; if (nx < 0 || ny < 0 || nx >= SZ || ny >= SZ) continue; if (closed[nx][ny] || grid.getCell(nx, ny).isWall) continue; int ng = gCost[cx][cy] + 1; if (ng < gCost[nx][ny]) { gCost[nx][ny] = ng;parent[nx][ny] = { cx,cy };pq.push(nx, ny, ng + Pathfinding::heuristic(nx, ny, gX, gY));if (!inFrontier[nx][ny] && !closed[nx][ny]) { inFrontier[nx][ny] = true;if (!visited[nx][ny])visited[nx][ny] = true;added.push_back({ nx,ny }); } } }
            emit({ cx,cy }, std::move(added), { {cx,cy} }); if (cx == gX && cy == gY) { found = true;break; }
        }
        if (found) { std::vector<std::pair<int, int>> fp; int x = gX, y = gY; while (!(x == startX && y == startY)) { fp.push_back({ x,y });auto p = parent[x][y];if (p.first == -1)break;x = p.first;y = p.second; } fp.push_back({ startX,startY }); std::reverse(fp.begin(), fp.end()); VizFrame last; last.finalPath = std::move(fp); vizFrames.push_back(std::move(last)); }
    }
}

// ═══════════════════════════════════════════════════════════
//  UI Helpers
// ═══════════════════════════════════════════════════════════
void Game::drawCenteredText(const std::string& s, unsigned sz, sf::Color col, float y)
{
    if (!fontLoaded) return; sf::Text t(font, s, sz); t.setFillColor(col); auto b = t.getLocalBounds(); t.setOrigin(sf::Vector2f(b.size.x / 2.f, 0.f)); t.setPosition(sf::Vector2f(WIN_W / 2.f, y)); window.draw(t);
}

void Game::drawPanel(float x, float y, float w, float h, sf::Color fill, sf::Color border, float)
{
    sf::RectangleShape p(sf::Vector2f(w, h)); p.setPosition(sf::Vector2f(x, y)); p.setFillColor(fill); if (border != sf::Color::Transparent) { p.setOutlineThickness(2.f);p.setOutlineColor(border); } window.draw(p);
}

sf::FloatRect Game::drawButton(const std::string& lbl, float x, float y, float w, float h, sf::Color fill, sf::Color tc)
{
    sf::Vector2i m = sf::Mouse::getPosition(window); bool hov = (m.x >= x && m.x <= x + w && m.y >= y && m.y <= y + h);
    sf::RectangleShape b(sf::Vector2f(w, h)); b.setPosition(sf::Vector2f(x, y));
    b.setFillColor(hov ? sf::Color(fill.r + 25, fill.g + 25, fill.b + 25) : fill);
    b.setOutlineThickness(hov ? 2.f : 1.f); b.setOutlineColor(hov ? sf::Color(255, 255, 255, 180) : sf::Color(fill.r + 50, fill.g + 50, fill.b + 50));
    window.draw(b);
    if (fontLoaded) { sf::Text t(font, lbl, 15); t.setFillColor(tc); auto bd = t.getLocalBounds(); t.setOrigin(sf::Vector2f(bd.size.x / 2.f, bd.size.y / 2.f)); t.setPosition(sf::Vector2f(x + w / 2.f, y + h / 2.f)); window.draw(t); }
    return sf::FloatRect(sf::Vector2f(x, y), sf::Vector2f(w, h));
}

void Game::drawInputField(const std::string& label, const std::string& val, bool active, bool masked, float x, float y, float w)
{
    if (!fontLoaded) return;
    float t = animClock.getElapsedTime().asSeconds();
    sf::Text lbl(font, label, 13); lbl.setFillColor(sf::Color(150, 155, 180)); lbl.setPosition(sf::Vector2f(x, y)); window.draw(lbl);
    sf::RectangleShape box(sf::Vector2f(w, 38.f)); box.setPosition(sf::Vector2f(x, y + 18)); box.setFillColor(sf::Color(12, 14, 28));
    float pulse = 0.5f + 0.5f * std::sin(t * 3.f);
    box.setOutlineThickness(active ? 2.f : 1.f);
    box.setOutlineColor(active ? sf::Color((uint8_t)(80 + pulse * 60), (uint8_t)(140 + pulse * 60), 255) : sf::Color(55, 60, 85));
    window.draw(box);
    std::string d = masked ? std::string(val.size(), '*') : val;
    if (active && (int)(t * 2.f) % 2 == 0) d += "_";
    sf::Text v(font, d, 18); v.setFillColor(active ? sf::Color(220, 225, 255) : sf::Color(160, 165, 190)); v.setPosition(sf::Vector2f(x + 10, y + 26)); window.draw(v);
}

// ═══════════════════════════════════════════════════════════
//  3-D-style drawing  (isometric shading with SFML 2D shapes)
//  Player: blue cube   |   Enemy: red/orange cube
// ═══════════════════════════════════════════════════════════
void Game::draw3DPlayer(float cx, float cy, float cs)
{
    float s = cs * 0.82f; float off = cs * 0.18f;

    // Shadow
    sf::RectangleShape sh(sf::Vector2f(s, s * 0.25f));
    sh.setFillColor(sf::Color(0, 0, 0, 60));
    sh.setPosition(sf::Vector2f(cx - s / 2.f + off * 0.5f, cy + s * 0.38f));
    window.draw(sh);

    // Top face (lightest)
    sf::ConvexShape top(4);
    top.setPoint(0, sf::Vector2f(cx, cy - s * 0.5f));
    top.setPoint(1, sf::Vector2f(cx + off, cy - s * 0.5f + off * 0.5f));
    top.setPoint(2, sf::Vector2f(cx, cy - s * 0.5f + off));
    top.setPoint(3, sf::Vector2f(cx - off, cy - s * 0.5f + off * 0.5f));
    top.setFillColor(sf::Color(120, 200, 255));
    window.draw(top);

    // Front face
    sf::RectangleShape front(sf::Vector2f(s * 0.5f, s * 0.5f));
    front.setFillColor(sf::Color(40, 130, 220));
    front.setPosition(sf::Vector2f(cx - s * 0.25f, cy - s * 0.5f + off));
    window.draw(front);

    // Right face (darker)
    sf::ConvexShape right(4);
    right.setPoint(0, sf::Vector2f(cx + s * 0.25f, cy - s * 0.5f + off));
    right.setPoint(1, sf::Vector2f(cx + s * 0.25f + off, cy - s * 0.5f + off + off * 0.5f));
    right.setPoint(2, sf::Vector2f(cx + s * 0.25f + off, cy + off * 0.5f));
    right.setPoint(3, sf::Vector2f(cx + s * 0.25f, cy));
    right.setFillColor(sf::Color(20, 80, 160));
    window.draw(right);
}

void Game::draw3DEnemy(float cx, float cy, float cs, bool chase)
{
    float s = cs * 0.82f; float off = cs * 0.18f;
    float t = animClock.getElapsedTime().asSeconds();
    float pulse = 0.5f + 0.5f * std::sin(t * 5.f);

    sf::Color topCol = chase ? sf::Color((uint8_t)(220 + pulse * 35), 80, 80) : sf::Color(220, (uint8_t)(150 + pulse * 40), 20);
    sf::Color frontCol = chase ? sf::Color(180, 40, 40) : sf::Color(170, 100, 10);
    sf::Color rightCol = chase ? sf::Color(120, 20, 20) : sf::Color(110, 60, 5);

    // Glow ring in chase mode
    if (chase)
    {
        sf::CircleShape glow(s * 0.62f);
        glow.setOrigin(sf::Vector2f(s * 0.62f, s * 0.62f));
        glow.setPosition(sf::Vector2f(cx, cy - s * 0.1f));
        glow.setFillColor(sf::Color(255, 60, 60, (uint8_t)(40 + pulse * 50)));
        window.draw(glow);
    }

    // Shadow
    sf::RectangleShape sh(sf::Vector2f(s, s * 0.25f));
    sh.setFillColor(sf::Color(0, 0, 0, 60));
    sh.setPosition(sf::Vector2f(cx - s / 2.f + off * 0.5f, cy + s * 0.38f));
    window.draw(sh);

    // Top face
    sf::ConvexShape top(4);
    top.setPoint(0, sf::Vector2f(cx, cy - s * 0.5f));
    top.setPoint(1, sf::Vector2f(cx + off, cy - s * 0.5f + off * 0.5f));
    top.setPoint(2, sf::Vector2f(cx, cy - s * 0.5f + off));
    top.setPoint(3, sf::Vector2f(cx - off, cy - s * 0.5f + off * 0.5f));
    top.setFillColor(topCol);
    window.draw(top);

    // Front face
    sf::RectangleShape front(sf::Vector2f(s * 0.5f, s * 0.5f));
    front.setFillColor(frontCol);
    front.setPosition(sf::Vector2f(cx - s * 0.25f, cy - s * 0.5f + off));
    window.draw(front);

    // Right face
    sf::ConvexShape right(4);
    right.setPoint(0, sf::Vector2f(cx + s * 0.25f, cy - s * 0.5f + off));
    right.setPoint(1, sf::Vector2f(cx + s * 0.25f + off, cy - s * 0.5f + off + off * 0.5f));
    right.setPoint(2, sf::Vector2f(cx + s * 0.25f + off, cy + off * 0.5f));
    right.setPoint(3, sf::Vector2f(cx + s * 0.25f, cy));
    right.setFillColor(rightCol);
    window.draw(right);

    // Eyes (two white dots)
    float eyeR = s * 0.07f;
    sf::CircleShape eye(eyeR);
    eye.setFillColor(sf::Color(255, 255, 255));
    eye.setOrigin(sf::Vector2f(eyeR, eyeR));
    eye.setPosition(sf::Vector2f(cx - s * 0.10f, cy - s * 0.18f + off));
    window.draw(eye);
    eye.setPosition(sf::Vector2f(cx + s * 0.10f, cy - s * 0.18f + off));
    window.draw(eye);
}

// ═══════════════════════════════════════════════════════════
//  Render dispatcher
// ═══════════════════════════════════════════════════════════
void Game::render()
{
    window.clear(sf::Color(10, 12, 22));
    switch (state)
    {
    case GameState::LOGIN:       renderLogin();       break;
    case GameState::SIGNUP:      renderSignup();      break;
    case GameState::START:       renderStartScreen(); break;
    case GameState::LEADERBOARD: renderLeaderboard(); break;
    case GameState::VISUALIZING: renderVisualizer();  break;
    case GameState::PAUSED:
        renderGrid(); renderHUD(); renderPauseOverlay(); break;
    case GameState::WIN:
    case GameState::LOSE:
        renderGrid(); renderHUD(); renderEndScreen(state == GameState::WIN); break;
    default:
        renderGrid(); renderHUD(); break;
    }
    window.display();
}

// ═══════════════════════════════════════════════════════════
//  LOGIN screen
// ═══════════════════════════════════════════════════════════
void Game::renderLogin()
{
    float t = animClock.getElapsedTime().asSeconds();
    sf::RectangleShape dot(sf::Vector2f(3, 3));
    for (int i = 0;i < 20;i++) for (int j = 0;j < 16;j++) {
        float p = 0.5f + 0.5f * std::sin(t * 1.2f + i * 0.4f + j * 0.5f);
        dot.setFillColor(sf::Color(30, 50, 110, (uint8_t)(30 + p * 50)));
        dot.setPosition(sf::Vector2f(18.f + i * ((WIN_W - 36.f) / 19.f), 18.f + j * ((WIN_H - 36.f) / 15.f)));
        window.draw(dot);
    }
    float pw = 440, ph = 360, px = (WIN_W - pw) / 2.f, py = (WIN_H - ph) / 2.f;
    drawPanel(px, py, pw, ph, sf::Color(12, 14, 28, 245), sf::Color(55, 80, 150));
    if (!fontLoaded) return;
    float pulse = 0.5f + 0.5f * std::sin(t * 2.f); uint8_t tv = (uint8_t)(180 + pulse * 75);
    drawCenteredText("TRAPSCAPE", 28, sf::Color(tv, 200, 255), py + 18);
    drawCenteredText("Sign In", 14, sf::Color(120, 130, 165), py + 54);
    float fx = px + 32, fw = pw - 64;
    drawInputField("Username", authUser, activeField == AuthField::USERNAME, false, fx, py + 92, fw);
    drawInputField("Password", authPw, activeField == AuthField::PASSWORD, !showPw, fx, py + 162, fw);
    if (!authMsg.empty()) drawCenteredText(authMsg, 13, authMsgGood ? sf::Color(60, 220, 120) : sf::Color(240, 80, 80), py + 218);
    float btnY = py + 240, btnH = 40, half = pw / 2.f - 40;
    drawButton("LOG IN", px + 32, btnY, half, btnH, sf::Color(30, 70, 180), sf::Color(255, 255, 255));
    drawButton("CREATE ACCOUNT", px + 32 + half + 16, btnY, half, btnH, sf::Color(20, 90, 55), sf::Color(255, 255, 255));
    drawCenteredText("TAB=switch  ENTER=login  F2=show password", 10, sf::Color(85, 90, 120), py + 292);
    sf::RectangleShape div(sf::Vector2f(pw * 0.84f, 1.f)); div.setFillColor(sf::Color(50, 60, 110)); div.setPosition(sf::Vector2f(px + pw * 0.08f, py + ph - 46)); window.draw(div);
    drawCenteredText("DSA: Hash Table  O(1) lookup  djb2 double hashing", 10, sf::Color(65, 70, 100), py + ph - 34);
}

// ═══════════════════════════════════════════════════════════
//  SIGNUP screen
// ═══════════════════════════════════════════════════════════
void Game::renderSignup()
{
    float t = animClock.getElapsedTime().asSeconds();
    sf::RectangleShape dot(sf::Vector2f(3, 3));
    for (int i = 0;i < 20;i++) for (int j = 0;j < 16;j++) {
        float p = 0.5f + 0.5f * std::sin(t * 1.2f + i * 0.4f + j * 0.5f);
        dot.setFillColor(sf::Color(30, 80, 60, (uint8_t)(30 + p * 50)));
        dot.setPosition(sf::Vector2f(18.f + i * ((WIN_W - 36.f) / 19.f), 18.f + j * ((WIN_H - 36.f) / 15.f)));
        window.draw(dot);
    }
    float pw = 440, ph = 440, px = (WIN_W - pw) / 2.f, py = (WIN_H - ph) / 2.f;
    drawPanel(px, py, pw, ph, sf::Color(12, 14, 28, 245), sf::Color(40, 110, 70));
    if (!fontLoaded) return;
    float pulse = 0.5f + 0.5f * std::sin(t * 2.f); uint8_t tv = (uint8_t)(140 + pulse * 115);
    drawCenteredText("Create Account", 24, sf::Color(60, tv, 100), py + 18);
    float fx = px + 32, fw = pw - 64;
    drawInputField("Username (max 16, letters/digits/_)", authUser, activeField == AuthField::USERNAME, false, fx, py + 90, fw);
    drawInputField("Password (min 4 characters)", authPw, activeField == AuthField::PASSWORD, !showPw, fx, py + 160, fw);
    drawInputField("Confirm Password", authConfirm, activeField == AuthField::CONFIRM, !showPw, fx, py + 230, fw);
    if (!authMsg.empty()) drawCenteredText(authMsg, 13, authMsgGood ? sf::Color(60, 220, 120) : sf::Color(240, 80, 80), py + 286);
    float btnY = py + 305, btnH = 40, half = pw / 2.f - 40;
    drawButton("CREATE ACCOUNT", px + 32, btnY, half, btnH, sf::Color(20, 90, 55), sf::Color(255, 255, 255));
    drawButton("BACK TO LOGIN", px + 32 + half + 16, btnY, half, btnH, sf::Color(50, 50, 90), sf::Color(200, 210, 255));
    drawCenteredText("TAB=next  ENTER=create  ESC=back", 10, sf::Color(85, 90, 120), py + 356);
}

// ═══════════════════════════════════════════════════════════
//  START SCREEN
// ═══════════════════════════════════════════════════════════
void Game::renderStartScreen()
{
    float t = animClock.getElapsedTime().asSeconds();

    // ── animated dot background ───────────────────────────────
    sf::RectangleShape dot(sf::Vector2f(4, 4));
    for (int i = 0;i < 18;i++) for (int j = 0;j < 14;j++) {
        float pulse = 0.5f + 0.5f * std::sin(t * 1.4f + i * 0.45f + j * 0.55f);
        dot.setFillColor(sf::Color(40, 100, 200, (uint8_t)(60 + pulse * 120)));
        dot.setPosition(sf::Vector2f(20.f + i * ((WIN_W - 40.f) / 17.f), 20.f + j * ((WIN_H - 40.f) / 13.f)));
        window.draw(dot);
    }

    // ── main panel ────────────────────────────────────────────
    float panW = WIN_W * 0.80f, panH = WIN_H * 0.90f;
    float panX = (WIN_W - panW) / 2.f, panY = (WIN_H - panH) / 2.f;
    drawPanel(panX, panY, panW, panH, sf::Color(14, 18, 38), sf::Color(50, 110, 230));
    if (!fontLoaded) return;

    // ── title ─────────────────────────────────────────────────
    float pulse = 0.5f + 0.5f * std::sin(t * 2.f);
    uint8_t rb = (uint8_t)(180 + pulse * 75);
    drawCenteredText("TRAPSCAPE", 40, sf::Color(rb, 210, 255), panY + 16);

    std::string welcome = auth.loggedIn()
        ? "Welcome back,   " + auth.username() + " !"
        : "DSA  AI  Project";
    drawCenteredText(welcome, 15, sf::Color(80, 220, 130), panY + 64);

    // ── divider ───────────────────────────────────────────────
    sf::RectangleShape div1(sf::Vector2f(panW * 0.82f, 2.f));
    div1.setFillColor(sf::Color(50, 100, 200, 180));
    div1.setPosition(sf::Vector2f(panX + panW * 0.09f, panY + 90));
    window.draw(div1);

    // ── CONTROLS BOX (now at top, below title) ────────────────
    float cby = panY + 98;
    sf::RectangleShape cbox(sf::Vector2f(panW * 0.82f, 68.f));
    cbox.setPosition(sf::Vector2f(panX + panW * 0.09f, cby));
    cbox.setFillColor(sf::Color(8, 10, 24));
    cbox.setOutlineThickness(1.f);
    cbox.setOutlineColor(sf::Color(40, 60, 140));
    window.draw(cbox);
    drawCenteredText("CONTROLS", 12, sf::Color(140, 150, 190), cby + 5);
    drawCenteredText("WASD / Arrows = Move     P = Pause     R = Restart", 11, sf::Color(175, 180, 205), cby + 22);
    drawCenteredText("V = BFS Visualizer     A = A* Visualizer     L = Leaderboard     ESC = Back", 11, sf::Color(175, 180, 205), cby + 40);

    // ── divider ───────────────────────────────────────────────
    sf::RectangleShape div2(sf::Vector2f(panW * 0.82f, 2.f));
    div2.setFillColor(sf::Color(50, 100, 200, 120));
    div2.setPosition(sf::Vector2f(panX + panW * 0.09f, panY + 175));
    window.draw(div2);

    // ── DSA features list ─────────────────────────────────────
    struct Item { const char* l; sf::Color c; };
    Item items[] = {
        {">>  31x31 DFS maze generation  [ Stack ]",        sf::Color(100,220,255)},
        {">>  BFS patrol  +  A* chase  [ Queue / Min-Heap ]",sf::Color(255,200,70)},
        {">>  AI path traversal  [ Linked List ]",          sf::Color(80,220,130)},
        {">>  Collectible coins  +  Heat map",              sf::Color(255,210,50)},
        {">>  Top-10 Leaderboard  [ Min-Heap ]",            sf::Color(255,140,60)},
        {">>  User Auth  [ Hash Table + djb2 ]",            sf::Color(200,130,255)},
        {">>  BFS / A* Visualizer  ( press V or A )",       sf::Color(160,200,255)},
    };
    float by = panY + 183;
    for (auto& item : items) { drawCenteredText(item.l, 13, item.c, by); by += 26; }

    // ── divider above buttons ─────────────────────────────────
    float divY = panY + panH - 118;
    sf::RectangleShape div3(sf::Vector2f(panW * 0.82f, 2.f));
    div3.setFillColor(sf::Color(50, 100, 200, 120));
    div3.setPosition(sf::Vector2f(panX + panW * 0.09f, divY));
    window.draw(div3);

    // ── blinking start prompt ─────────────────────────────────
    if ((int)(t * 2.2f) % 2 == 0)
        drawCenteredText("PRESS  ENTER  OR  SPACE  TO  START", 15, sf::Color(60, 230, 150), divY + 8);

    // ── three buttons: START  LEADERBOARD  LOGOUT ─────────────
    // Each button is evenly spaced inside the panel
    float btnH = 40.f, btnW = panW * 0.26f;
    float totalBtnW = btnW * 3 + 24.f;          // 3 buttons + 2 gaps of 12
    float btnStartX = panX + (panW - totalBtnW) / 2.f;
    float btnY = panY + panH - 62.f;

    drawButton("START  GAME", btnStartX, btnY, btnW, btnH, sf::Color(25, 75, 185), sf::Color(255, 255, 255));
    drawButton("LEADERBOARD", btnStartX + btnW + 12.f, btnY, btnW, btnH, sf::Color(130, 80, 10), sf::Color(255, 255, 255));
    drawButton("LOGOUT", btnStartX + btnW * 2 + 24.f, btnY, btnW, btnH, sf::Color(120, 25, 25), sf::Color(255, 255, 255));
}

// ═══════════════════════════════════════════════════════════
//  GRID  —  now uses 3-D player and enemy
// ═══════════════════════════════════════════════════════════
void Game::renderGrid()
{
    float cs = (float)CELL_SIZE;
    sf::RectangleShape cell(sf::Vector2f(cs - 1, cs - 1));

    for (int col = 0;col < Grid::SIZE;col++)
        for (int row = 0;row < Grid::SIZE;row++) {
            if (grid.getCell(col, row).isWall) { cell.setFillColor(sf::Color(45, 50, 70)); }
            else { int heat = heatMap[col][row]; int r = 22 + std::min(heat * 12, 100); cell.setFillColor(sf::Color(r, 22, 35)); }
            cell.setPosition(sf::Vector2f(col * cs + OFFSET_X, row * cs + OFFSET_Y));
            window.draw(cell);
        }

    // AI ghost path dots nlues dots show
    PathNode* node = aiPath.getHead();
    while (node) { sf::RectangleShape dot(sf::Vector2f(4, 4)); dot.setFillColor(sf::Color(0, 140, 255, 100)); dot.setPosition(sf::Vector2f(node->x * cs + OFFSET_X + cs / 2 - 2, node->y * cs + OFFSET_Y + cs / 2 - 2)); window.draw(dot); node = node->next; }

    // Coins (pulsing yellow circles)
    float t = animClock.getElapsedTime().asSeconds();
    float coinPulse = 0.7f + 0.3f * std::sin(t * 4.f);
    for (int row = 0;row < Grid::SIZE;row++)
        for (int col = 0;col < Grid::SIZE;col++)
            if (coinGrid[row][col])
            {
                float r = cs * 0.18f * coinPulse;
                sf::CircleShape coin(r); coin.setOrigin(sf::Vector2f(r, r));
                coin.setFillColor(sf::Color(255, 215, 30));
                coin.setOutlineThickness(1.f); coin.setOutlineColor(sf::Color(255, 255, 100, 180));
                coin.setPosition(sf::Vector2f(col * cs + OFFSET_X + cs / 2.f, row * cs + OFFSET_Y + cs / 2.f));
                window.draw(coin);
            }

    // Goal — pulsing green
    float pulse = 0.5f + 0.5f * std::sin(t * 3.f); uint8_t gb = (uint8_t)(140 + pulse * 115);
    sf::RectangleShape goal(sf::Vector2f(cs - 4, cs - 4));
    goal.setFillColor(sf::Color(0, gb, 60)); goal.setOutlineThickness(1.5f); goal.setOutlineColor(sf::Color(80, 255, 130, (uint8_t)(130 + pulse * 125)));
    goal.setPosition(sf::Vector2f(goalX * cs + OFFSET_X + 2, goalY * cs + OFFSET_Y + 2)); window.draw(goal);

    // ── 3-D Player ────────────────────────────────────────────
    float pcx = player.getX() * cs + OFFSET_X + cs / 2.f;
    float pcy = player.getY() * cs + OFFSET_Y + cs / 2.f;
    draw3DPlayer(pcx, pcy, cs);

    // ── 3-D Enemy ─────────────────────────────────────────────
    float acx = aiX * cs + OFFSET_X + cs / 2.f;
    float acy = aiY * cs + OFFSET_Y + cs / 2.f;
    draw3DEnemy(acx, acy, cs, aiMode == AIMode::CHASE);
}

// ═══════════════════════════════════════════════════════════
//  HUD  — now shows coins
// ═══════════════════════════════════════════════════════════
void Game::renderHUD()
{
    float barY = Grid::SIZE * CELL_SIZE + OFFSET_Y + 4, barW = WIN_W - OFFSET_X * 2;
    drawPanel(OFFSET_X, barY, barW, HUD_HEIGHT - 8, sf::Color(18, 20, 38), sf::Color(40, 55, 110));
    if (!fontLoaded) return;

    std::string modeStr = (aiMode == AIMode::CHASE) ? "AI: CHASE [A*]" : "AI: PATROL [BFS]";
    sf::Color   modeCol = (aiMode == AIMode::CHASE) ? sf::Color(255, 90, 90) : sf::Color(255, 195, 50);
    sf::Text m(font, modeStr, 14); m.setFillColor(modeCol); m.setPosition(sf::Vector2f(OFFSET_X + 8, barY + 6)); window.draw(m);

    sf::Text sc(font, "Score: " + std::to_string(player.getScore()) + "   Best: " + std::to_string(bestScore), 13);
    sc.setFillColor(sf::Color(100, 220, 255)); sc.setPosition(sf::Vector2f(OFFSET_X + 8, barY + 28)); window.draw(sc);

    sf::Text coins(font, "Coins: " + std::to_string(coinsCollected) + "/" + std::to_string(coinsTotal), 13);
    coins.setFillColor(sf::Color(255, 215, 40)); coins.setPosition(sf::Vector2f(OFFSET_X + 260, barY + 6)); window.draw(coins);

    sf::Text hint(font, "WASD=Move  P=Pause  V=BFS  A=A*  R=Restart  L=Board", 11);
    hint.setFillColor(sf::Color(100, 108, 140));
    auto hb = hint.getLocalBounds(); hint.setPosition(sf::Vector2f(WIN_W - OFFSET_X - hb.size.x - 4, barY + 6)); window.draw(hint);

    sf::Text dist(font, "AI dist: " + std::to_string(manhattanToPlayer()), 13);
    dist.setFillColor(sf::Color(155, 155, 180));
    auto db = dist.getLocalBounds(); dist.setPosition(sf::Vector2f(WIN_W - OFFSET_X - db.size.x - 4, barY + 28)); window.draw(dist);
}

// ═══════════════════════════════════════════════════════════
//  VISUALIZER (unchanged from v2)
// ═══════════════════════════════════════════════════════════
void Game::renderVisualizer()
{
    if (vizFrames.empty()) return;
    int fi = std::min(vizFrameIdx, (int)vizFrames.size() - 1);
    if (fi < vizLastReplayIdx) { vizVisited.clear(); vizFrontier.clear(); vizLastReplayIdx = -1; }
    for (int f = vizLastReplayIdx + 1;f <= fi;f++) {
        const VizFrame& vf = vizFrames[f];
        if (vf.newVisited.first >= 0) vizVisited.push_back(vf.newVisited);
        for (auto& rc : vf.removedFrontier) for (int k = 0;k < (int)vizFrontier.size();k++) if (vizFrontier[k] == rc) { vizFrontier[k] = vizFrontier.back();vizFrontier.pop_back();break; }
        for (auto& nc : vf.newFrontier) vizFrontier.push_back(nc);
    }
    vizLastReplayIdx = fi;
    sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
    for (int col = 0;col < Grid::SIZE;col++) for (int row = 0;row < Grid::SIZE;row++) { cell.setFillColor(grid.getCell(col, row).isWall ? sf::Color(45, 50, 70) : sf::Color(18, 18, 30)); cell.setPosition(sf::Vector2f(col * CELL_SIZE + OFFSET_X, row * CELL_SIZE + OFFSET_Y)); window.draw(cell); }
    for (auto& p : vizVisited) { cell.setFillColor(sf::Color(30, 90, 210, 210)); cell.setPosition(sf::Vector2f(p.first * CELL_SIZE + OFFSET_X, p.second * CELL_SIZE + OFFSET_Y)); window.draw(cell); }
    for (auto& p : vizFrontier) { cell.setFillColor(sf::Color(240, 195, 0, 230)); cell.setPosition(sf::Vector2f(p.first * CELL_SIZE + OFFSET_X, p.second * CELL_SIZE + OFFSET_Y)); window.draw(cell); }
    for (auto& p : vizFrames[fi].finalPath) { cell.setFillColor(sf::Color(20, 240, 100)); cell.setPosition(sf::Vector2f(p.first * CELL_SIZE + OFFSET_X, p.second * CELL_SIZE + OFFSET_Y)); window.draw(cell); }
    sf::RectangleShape goal(sf::Vector2f(CELL_SIZE - 4, CELL_SIZE - 4)); goal.setFillColor(sf::Color(0, 200, 80)); goal.setPosition(sf::Vector2f(goalX * CELL_SIZE + OFFSET_X + 2, goalY * CELL_SIZE + OFFSET_Y + 2)); window.draw(goal);
    float barY = Grid::SIZE * CELL_SIZE + OFFSET_Y + 4;
    drawPanel(OFFSET_X, barY, WIN_W - OFFSET_X * 2, HUD_HEIGHT - 8, sf::Color(14, 16, 30), sf::Color(40, 55, 110));
    if (fontLoaded) {
        std::string algo = vizUseBFS ? "BFS Visualizer  [ Queue ]" : "A* Visualizer  [ Min-Heap ]";
        sf::Color acol = vizUseBFS ? sf::Color(80, 180, 255) : sf::Color(255, 150, 50);
        sf::Text title(font, algo + "   Step " + std::to_string(fi + 1) + " / " + std::to_string((int)vizFrames.size()), 15); title.setFillColor(acol); title.setPosition(sf::Vector2f(OFFSET_X + 8, barY + 6)); window.draw(title);
        sf::Text leg(font, "Blue=Visited   Yellow=Frontier   Green=Shortest Path   |   ESC=Back", 12); leg.setFillColor(sf::Color(130, 138, 160)); leg.setPosition(sf::Vector2f(OFFSET_X + 8, barY + 28)); window.draw(leg);
    }
}

// ═══════════════════════════════════════════════════════════
//  END SCREEN  — now shows coins
// ═══════════════════════════════════════════════════════════
void Game::renderEndScreen(bool won)
{
    float t = animClock.getElapsedTime().asSeconds();
    sf::RectangleShape ov(sf::Vector2f((float)WIN_W, (float)WIN_H)); ov.setFillColor(sf::Color(0, 0, 0, 165)); window.draw(ov);
    float panW = WIN_W * 0.62f, panH = WIN_H * 0.56f, panX = (WIN_W - panW) / 2.f, panY = (WIN_H - panH) / 2.f;
    sf::Color border = won ? sf::Color(50, 220, 100, 230) : sf::Color(220, 55, 55, 230);
    drawPanel(panX, panY, panW, panH, sf::Color(10, 12, 28, 240), border);
    if (!fontLoaded) return;
    float pulse = 0.5f + 0.5f * std::sin(t * 2.5f);
    if (won) { uint8_t gv = (uint8_t)(170 + pulse * 85); drawCenteredText("YOU ESCAPED!", 42, sf::Color(50, gv, 90), panY + 28); drawCenteredText("The AI couldn't catch you!", 17, sf::Color(150, 220, 170), panY + 82); }
    else { uint8_t rv = (uint8_t)(170 + pulse * 85); drawCenteredText("CAUGHT!", 46, sf::Color(rv, 45, 45), panY + 28);    drawCenteredText("The AI hunted you down.", 17, sf::Color(220, 140, 140), panY + 82); }
    sf::RectangleShape div(sf::Vector2f(panW * 0.78f, 1.f)); div.setFillColor(sf::Color(70, 80, 130)); div.setPosition(sf::Vector2f(panX + panW * 0.11f, panY + 120)); window.draw(div);
    drawCenteredText("Score: " + std::to_string(player.getScore()), 19, sf::Color(110, 210, 255), panY + 132);
    drawCenteredText("Coins: " + std::to_string(coinsCollected) + " / " + std::to_string(coinsTotal), 16, sf::Color(255, 215, 40), panY + 162);
    drawCenteredText("Moves: " + std::to_string(moveCount), 16, sf::Color(175, 180, 210), panY + 190);
    drawCenteredText("Best: " + std::to_string(bestScore), 14, sf::Color(160, 135, 225), panY + 216);
    float bw = panW * 0.36f, bh = 40, gap = (panW - 2 * bw) / 3.f, by = panY + panH - 56;
    drawButton(won ? "PLAY AGAIN" : "TRY AGAIN", panX + gap, by, bw, bh, sf::Color(25, 75, 185), sf::Color(255, 255, 255));
    drawButton("LEADERBOARD", panX + gap * 2 + bw, by, bw, bh, sf::Color(130, 80, 10), sf::Color(255, 255, 255));
}

// ═══════════════════════════════════════════════════════════
//  PAUSE OVERLAY
// ═══════════════════════════════════════════════════════════
void Game::renderPauseOverlay()
{
    float t = animClock.getElapsedTime().asSeconds();

    // Dim background
    sf::RectangleShape ov(sf::Vector2f((float)WIN_W, (float)WIN_H));
    ov.setFillColor(sf::Color(0, 0, 0, 170));
    window.draw(ov);

    // Panel — must match click coords: pw=360 ph=240
    float pw = 360, ph = 240, px = (WIN_W - pw) / 2.f, py = (WIN_H - ph) / 2.f;
    drawPanel(px, py, pw, ph, sf::Color(10, 12, 30, 250), sf::Color(60, 100, 220));
    if (!fontLoaded) return;

    float pulse = 0.5f + 0.5f * std::sin(t * 2.5f);
    uint8_t pv = (uint8_t)(160 + pulse * 95);
    drawCenteredText("PAUSED", 26, sf::Color(pv, 180, 255), py + 14);

    float bw = pw * 0.65f, bh = 42, bx = px + (pw - bw) / 2.f;

    // Resume  (py+56)
    drawButton("RESUME  [ P ]", bx, py + 56, bw, bh, sf::Color(25, 75, 185), sf::Color(255, 255, 255));
    // Logout  (py+108)
    drawButton("LOGOUT", bx, py + 108, bw, bh, sf::Color(130, 30, 30), sf::Color(255, 255, 255));
    // Leaderboard  (py+160)
    drawButton("LEADERBOARD", bx, py + 160, bw, bh, sf::Color(130, 80, 10), sf::Color(255, 255, 255));

    // Current score
    drawCenteredText("Score: " + std::to_string(player.getScore()) + "   Moves: " + std::to_string(moveCount),
        12, sf::Color(120, 130, 170), py + ph - 16);
}

// ═══════════════════════════════════════════════════════════
//  LEADERBOARD screen
// ═══════════════════════════════════════════════════════════
void Game::renderLeaderboard()
{
    float t = animClock.getElapsedTime().asSeconds();
    sf::RectangleShape dot(sf::Vector2f(4, 4));
    for (int i = 0;i < 18;i++) for (int j = 0;j < 14;j++) {
        float p = 0.5f + 0.5f * std::sin(t * 1.2f + i * 0.4f + j * 0.5f);
        dot.setFillColor(sf::Color(200, 140, 20, (uint8_t)(40 + p * 80)));
        dot.setPosition(sf::Vector2f(20.f + i * ((WIN_W - 40.f) / 17.f), 20.f + j * ((WIN_H - 40.f) / 13.f)));
        window.draw(dot);
    }
    float panW = WIN_W * 0.82f, panH = WIN_H * 0.88f, panX = (WIN_W - panW) / 2.f, panY = (WIN_H - panH) / 2.f;
    drawPanel(panX, panY, panW, panH, sf::Color(10, 12, 28), sf::Color(200, 155, 30));
    if (!fontLoaded) return;

    float pulse = 0.5f + 0.5f * std::sin(t * 2.f); uint8_t tv = (uint8_t)(180 + pulse * 75);
    drawCenteredText("LEADERBOARD", 32, sf::Color(255, tv, 30), panY + 14);
    drawCenteredText("Top 10  —  Min-Heap  [ O(log N) insert ]", 13, sf::Color(160, 130, 60), panY + 54);

    sf::RectangleShape div(sf::Vector2f(panW * 0.88f, 1.f)); div.setFillColor(sf::Color(120, 90, 20, 180)); div.setPosition(sf::Vector2f(panX + panW * 0.06f, panY + 80)); window.draw(div);

    float c1 = panX + panW * 0.06f, c2 = panX + panW * 0.17f, c3 = panX + panW * 0.58f, c4 = panX + panW * 0.72f, c5 = panX + panW * 0.84f;
    float sy = panY + 88, rh = 30;
    auto dat = [&](const std::string& s, float x, float y, unsigned sz, sf::Color c) { sf::Text tx(font, s, sz); tx.setFillColor(c); tx.setPosition(sf::Vector2f(x, y)); window.draw(tx); };
    sf::Color hdr(140, 130, 80);
    dat("Rank", c1, sy, 12, hdr); dat("Name", c2, sy, 12, hdr); dat("Score", c3, sy, 12, hdr); dat("Moves", c4, sy, 12, hdr); dat("Coins", c5, sy, 12, hdr);
    sy += 18;
    sf::RectangleShape hl(sf::Vector2f(panW * 0.88f, 1.f)); hl.setFillColor(sf::Color(80, 65, 20, 140)); hl.setPosition(sf::Vector2f(panX + panW * 0.06f, sy)); window.draw(hl); sy += 4;

    LBEntry sorted[Leaderboard::TOP]; int cnt = 0;
    leaderboard.getSorted(sorted, cnt);
    sf::Color medals[3] = { sf::Color(255,210,40),sf::Color(200,200,200),sf::Color(205,127,50) };

    if (cnt == 0) { drawCenteredText("No scores yet — win the maze to appear here!", 14, sf::Color(120, 125, 155), panY + panH * 0.5f); }
    else for (int i = 0;i < cnt;i++) {
        float ry = sy + i * rh;
        if (i < 3) { sf::RectangleShape row(sf::Vector2f(panW * 0.88f, rh - 4)); row.setPosition(sf::Vector2f(panX + panW * 0.06f, ry)); row.setFillColor(sf::Color(50, 40, 10, 80)); window.draw(row); }
        sf::Color nc = (i < 3) ? medals[i] : sf::Color(175, 175, 190);
        dat("#" + std::to_string(i + 1), c1, ry + 4, 13, nc);
        dat(sorted[i].name, c2, ry + 4, 13, nc);
        dat(std::to_string(sorted[i].score), c3, ry + 4, 13, nc);
        dat(std::to_string(sorted[i].moves), c4, ry + 4, 12, sf::Color(130, 135, 160));
        dat(std::to_string(sorted[i].coins), c5, ry + 4, 12, sf::Color(255, 215, 40));
    }

    sf::RectangleShape fd(sf::Vector2f(panW * 0.88f, 1.f)); fd.setFillColor(sf::Color(80, 65, 20, 120)); fd.setPosition(sf::Vector2f(panX + panW * 0.06f, panY + panH - 66)); window.draw(fd);
    drawCenteredText("Min-Heap root = lowest score in top-10  |  insert/evict = O(log N)", 11, sf::Color(110, 95, 45), panY + panH - 58);
    drawButton("BACK", (WIN_W - 200) / 2.f, panY + panH - 44, 200, 32, sf::Color(50, 55, 90), sf::Color(200, 210, 255));
}