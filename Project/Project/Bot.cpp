#include "Bot.h"
#include <cmath>
#include <cstdlib>

static const float BOT_PI = 3.14159265f;

static float RandRange(float lo, float hi) {
    return lo + (hi - lo) * ((float)rand() / (float)RAND_MAX);
}

// Heading in degrees using the same convention as Game::UpdatePlayer:
// forward = (cos(rotation), -sin(rotation)) on the XZ plane.
static float HeadingOf(float dx, float dz) {
    return atan2f(-dz, dx) * 180.0f / BOT_PI;
}

// Signed shortest difference (target - current) in the range [-180, 180].
static float AngleDiff(float target, float current) {
    float d = fmodf(target - current, 360.0f);
    if (d > 180.0f)  d -= 360.0f;
    if (d < -180.0f) d += 360.0f;
    return d;
}

BotController::BotController() {
    Reset();
}

void BotController::Reset() {
    reactionTimer = BOT_ROUND_START_DELAY;
    hasDecision = false;
    desiredHeading = 0.0f;
    wantThrust = false;
    wantShoot = false;
    targetDist = 0.0f;
    aimError = RandRange(-BOT_AIM_ERROR_DEG, BOT_AIM_ERROR_DEG);
    triggerDelay = 0.0f;
    wanderTimer = RandRange(BOT_WANDER_INTERVAL_MIN, BOT_WANDER_INTERVAL_MAX);
    burstTimer = 0.0f;
}

void BotController::Decide(const Player& self, const Player& enemy,
    const std::vector<Asteroid>& asteroids) {

    // 1. Evade the asteroid that would hit us soonest.
    //    Uses the same collision radius as the asteroid-vs-player check.
    const Asteroid* threat = nullptr;
    float soonest = BOT_EVADE_TIME;
    for (const Asteroid& a : asteroids) {
        float dx = a.x - self.x, dz = a.z - self.z;
        float dist = sqrtf(dx * dx + dz * dz);
        float gap = dist - (a.radius * 1.5f + PLAYER_RADIUS);
        // Speed at which the gap is shrinking (relative velocity along the line between us)
        float closingSpeed = 0.0f;
        if (dist > 0.001f)
            closingSpeed = -((a.vx - self.vx) * dx + (a.vz - self.vz) * dz) / dist;

        float timeToImpact;
        if (gap < BOT_EVADE_GAP)       timeToImpact = 0.0f;
        else if (closingSpeed > 0.0f)  timeToImpact = gap / closingSpeed;
        else                           continue;

        if (timeToImpact < soonest) {
            threat = &a;
            soonest = timeToImpact;
        }
    }

    if (threat && self.invulnTime <= 0.0f) {
        float awayX = self.x - threat->x, awayZ = self.z - threat->z;
        float awayLen = sqrtf(awayX * awayX + awayZ * awayZ);
        if (awayLen > 0.001f) { awayX /= awayLen; awayZ /= awayLen; }

        // Also step sideways out of the asteroid's path (perpendicular to its
        // relative velocity, on whichever side we are already on).
        float wx = threat->vx - self.vx, wz = threat->vz - self.vz;
        float wLen = sqrtf(wx * wx + wz * wz);
        float sideX = 0.0f, sideZ = 0.0f;
        if (wLen > 0.001f) {
            sideX = -wz / wLen;
            sideZ = wx / wLen;
            if (sideX * awayX + sideZ * awayZ < 0.0f) { sideX = -sideX; sideZ = -sideZ; }
        }

        desiredHeading = HeadingOf(awayX + sideX, awayZ + sideZ);
        wantThrust = true;
        wantShoot = false;
        return;
    }

    // 2. Pick a target: the enemy whenever it can be hurt (chasing it if it is far away).
    //    While it is invulnerable after a hit, farm the nearest asteroid for points instead.
    float ex = enemy.x - self.x, ez = enemy.z - self.z;
    float enemyDist = sqrtf(ex * ex + ez * ez);
    bool enemyHittable = enemy.alive && enemy.invulnTime <= 0.0f;

    const Asteroid* rock = nullptr;
    float rockDist = BOT_FIRE_RANGE;
    for (const Asteroid& a : asteroids) {
        float dx = a.x - self.x, dz = a.z - self.z;
        float dist = sqrtf(dx * dx + dz * dz);
        float gap = dist - (a.radius * 1.5f + PLAYER_RADIUS);
        if (gap > BOT_MIN_ASTEROID_GAP && dist < rockDist) {
            rock = &a;
            rockDist = dist;
        }
    }

    float tx, tz, tvx, tvz;
    bool targetIsEnemy;
    if (enemyHittable) {
        tx = enemy.x; tz = enemy.z; tvx = enemy.vx; tvz = enemy.vz;
        targetIsEnemy = true;
        wantShoot = true; // Think() still waits until it is within BOT_FIRE_RANGE
    }
    else if (rock) {
        tx = rock->x; tz = rock->z; tvx = rock->vx; tvz = rock->vz;
        targetIsEnemy = false;
        wantShoot = true;
    }
    else if (enemy.alive) {
        tx = enemy.x; tz = enemy.z; tvx = enemy.vx; tvz = enemy.vz;
        targetIsEnemy = true;
        wantShoot = false;
    }
    else {
        wantThrust = false;
        wantShoot = false;
        return;
    }

    // 3. Aim with an imperfect lead (projectiles fly straight at PROJECTILE_SPEED).
    float dx = tx - self.x, dz = tz - self.z;
    targetDist = sqrtf(dx * dx + dz * dz);
    float flightTime = targetDist / PROJECTILE_SPEED;
    float lead = RandRange(BOT_LEAD_MIN, BOT_LEAD_MAX);
    float aimX = dx + tvx * flightTime * lead;
    float aimZ = dz + tvz * flightTime * lead;
    desiredHeading = HeadingOf(aimX, aimZ) + aimError;

    // Close the distance to the enemy, but don't fly into asteroids on purpose.
    wantThrust = targetIsEnemy && enemyDist > BOT_CHASE_DISTANCE;
}

InputState BotController::Think(const Player& self, const Player& enemy,
    const std::vector<Asteroid>& asteroids, float dt) {

    InputState inp;
    inp.playerId = self.id;
    inp.thrustForward = false;
    inp.rotateLeft = false;
    inp.rotateRight = false;
    inp.shoot = false;
    if (!self.alive) return inp;

    triggerDelay -= dt;
    reactionTimer -= dt;
    if (reactionTimer <= 0.0f) {
        Decide(self, enemy, asteroids);
        hasDecision = true;
        reactionTimer = RandRange(BOT_REACTION_MIN, BOT_REACTION_MAX);
    }
    if (!hasDecision) return inp;

    float diff = AngleDiff(desiredHeading, self.rotation);
    if (diff > BOT_TURN_DEADZONE_DEG)       inp.rotateLeft = true;
    else if (diff < -BOT_TURN_DEADZONE_DEG) inp.rotateRight = true;

    // Occasional short thrust bursts so the bot never sits perfectly still.
    if (burstTimer > 0.0f) {
        burstTimer -= dt;
    }
    else {
        wanderTimer -= dt;
        if (wanderTimer <= 0.0f) {
            burstTimer = RandRange(BOT_WANDER_BURST_MIN, BOT_WANDER_BURST_MAX);
            wanderTimer = RandRange(BOT_WANDER_INTERVAL_MIN, BOT_WANDER_INTERVAL_MAX);
        }
    }

    inp.thrustForward = (wantThrust && fabsf(diff) < BOT_THRUST_ANGLE_DEG) || burstTimer > 0.0f;

    if (wantShoot && fabsf(diff) < BOT_FIRE_ANGLE_DEG && targetDist < BOT_FIRE_RANGE &&
        self.shootCooldown <= 0.0f && triggerDelay <= 0.0f) {
        inp.shoot = true;
        triggerDelay = SHOOT_COOLDOWN + RandRange(0.0f, BOT_FIRE_HESITATION);
        aimError = RandRange(-BOT_AIM_ERROR_DEG, BOT_AIM_ERROR_DEG);
    }

    return inp;
}
