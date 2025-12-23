#pragma once

// Tipos de colision para identificar la reaccion
enum class CollisionType {
    None,
    Player,
    Enemy,
    Spike,
    Item
};

enum class EnemyAIState {
    Patrol,
    Chase,
    Retreat
};
