#pragma once

// World
const float WORLD_SIZE = 35.0f;

// Player
const float PLAYER_SPAWN_X = 25.0f;
const float PLAYER_ROTATION_SPEED = 180.0f;
const float PLAYER_ACCELERATION = 20.0f;
const float PLAYER_MAX_SPEED = 14.0f;
const float PLAYER_DRAG = 2.0f;
const float PLAYER_RADIUS = 1.0f;
const int   PLAYER_START_LIVES = 3;
const float PLAYER_INVULN_TIME = 1.5f;
const float PLAYER_FLASH_RATE = 0.1f;

// Projectile
const float PROJECTILE_SPEED = 30.0f;
const float SHOOT_COOLDOWN = 0.6f;
const float PROJECTILE_RADIUS = 0.1f;

// Asteroid
const float ASTEROID_BIG_RADIUS = 5.0f;
const float ASTEROID_MID_RADIUS = 3.0f;
const float ASTEROID_SMALL_RADIUS = 1.5f;
const float ASTEROID_BIG_SPEED = 2.0f;
const float ASTEROID_MID_SPEED = 3.5f;
const float ASTEROID_SMALL_SPEED = 5.0f;
const int   ASTEROID_INITIAL_COUNT = 4;
const int   ASTEROID_MIN_COUNT = 4;
const int   ASTEROID_MAX_COUNT = 25;
const float ASTEROID_SPAWN_INTERVAL = 2.5f;
const float ASTEROID_CENTER_ZONE = 12.0f;
const float ASTEROID_MIN_PLAYER_DIST = 10.0f;
const float ASTEROID_MIN_SPACING = 6.0f;

// Scoring
const int   SCORE_BIG_SPLIT = 5;
const int   SCORE_MID_SPLIT = 10;
const int   SCORE_SMALL_DESTROY = 20;

// Match
const float ROUND_TIME = 30.0f;
const int   ROUNDS_TO_WIN = 2;
const float ROUND_END_PAUSE = 3.0f;

// Bot (VS Bot mode) - all difficulty tuning lives here.
// The bot uses the same InputState and gameplay rules as a human player.
const float BOT_ROUND_START_DELAY  = 0.6f;  // seconds before the bot reacts at round start
const float BOT_REACTION_MIN       = 0.15f; // seconds between decisions (reaction time)
const float BOT_REACTION_MAX       = 0.30f;
const float BOT_AIM_ERROR_DEG      = 7.0f;  // max random aim offset, re-rolled after each shot
const float BOT_LEAD_MIN           = 0.3f;  // fraction of a perfect lead on a moving target
const float BOT_LEAD_MAX           = 1.0f;
const float BOT_TURN_DEADZONE_DEG  = 3.0f;  // stop turning when this close to the aim heading
const float BOT_FIRE_ANGLE_DEG     = 6.0f;  // only shoot when this close to the aim heading
const float BOT_FIRE_RANGE         = 32.0f;
const float BOT_FIRE_HESITATION    = 0.4f;  // max extra random delay on top of SHOOT_COOLDOWN
const float BOT_THRUST_ANGLE_DEG   = 40.0f; // only thrust when roughly facing where it wants to go
const float BOT_CHASE_DISTANCE     = 20.0f; // thrust towards the player when further than this
const float BOT_EVADE_GAP          = 2.0f;  // always evade an asteroid this close (surface distance)
const float BOT_EVADE_TIME         = 0.9f;  // evade when an asteroid would reach us within this time
const float BOT_MIN_ASTEROID_GAP   = 3.0f;  // don't shoot asteroids closer than this (debris)
const float BOT_WANDER_INTERVAL_MIN = 1.5f; // random thrust bursts so the bot never sits still
const float BOT_WANDER_INTERVAL_MAX = 3.5f;
const float BOT_WANDER_BURST_MIN   = 0.25f;
const float BOT_WANDER_BURST_MAX   = 0.6f;