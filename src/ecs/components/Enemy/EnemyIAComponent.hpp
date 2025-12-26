#pragma once
#include "ecs/Ecstypes.hpp"

struct EnemyAIComponent {
    EnemyAIState state = EnemyAIState::Patrol;
    float detectionRange = 6.0f;
    float retreatTimer = 0.0f;
    float retreatDuration = 3.0f;
    float moveCooldown = 0.0f;
    float timer = 0.0f;
};
