#pragma once
#include <vector>
#include "Entities.h"

// Drives one ship in VS Bot mode by producing the same InputState a human
// would produce with the keyboard. It only reads game state (no cheating):
// the gameplay code in Game::UpdatePlaying applies its input like any other.
class BotController {
public:
    BotController();

    // Call at the start of every round.
    void Reset();

    // Decide this frame's input for 'self'.
    InputState Think(const Player& self, const Player& enemy,
        const std::vector<Asteroid>& asteroids, float dt);

private:
    void Decide(const Player& self, const Player& enemy,
        const std::vector<Asteroid>& asteroids);

    float reactionTimer;   // time until the next decision
    bool  hasDecision;     // false until the first decision of the round
    float desiredHeading;  // degrees, chosen at the last decision
    bool  wantThrust;
    bool  wantShoot;       // current target is worth shooting at
    float targetDist;
    float aimError;        // degrees, re-rolled after each shot
    float triggerDelay;    // hesitation before the next shot is allowed
    float wanderTimer;     // time until the next random thrust burst
    float burstTimer;      // remaining time of the current burst
};
