#pragma once

// ── Grid ─────────────────────────────────────────────────────
const int SIZE      = 31;          // bigger maze (was 21) — must be ODD
const int CELL_SIZE = 22;          // slightly smaller cells to fit screen
const int OFFSET_X  = 16;
const int OFFSET_Y  = 16;

// ── Window ────────────────────────────────────────────────────
const int HUD_HEIGHT = 64;
const int WIN_W = SIZE * CELL_SIZE + OFFSET_X * 2;
const int WIN_H = SIZE * CELL_SIZE + OFFSET_Y * 2 + HUD_HEIGHT;

// ── AI behaviour ──────────────────────────────────────────────
const int   CHASE_THRESHOLD  = 6;     // Manhattan dist — switch BFS→A*
const float AI_MOVE_DELAY    = 0.32f; // seconds per AI step (was 0.15 — fairer)
const float VIZ_STEP_DELAY   = 0.025f;

// ── AI handicap ───────────────────────────────────────────────
// Recalculate AI path only every N player moves
const int AI_REPATH_INTERVAL = 3;

// ── Scoring ───────────────────────────────────────────────────
const int SCORE_PER_MOVE  = 1;
const int SCORE_WIN_BONUS = 500;
