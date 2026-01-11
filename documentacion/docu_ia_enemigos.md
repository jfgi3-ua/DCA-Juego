# Documentación: IA de Enemigos

## Índice
1. [Introducción](#introducción)
2. [Arquitectura General](#arquitectura-general)
3. [Estados de la IA](#estados-de-la-ia)
4. [Árbol de Decisiones](#árbol-de-decisiones)
5. [Comportamientos Específicos](#comportamientos-específicos)
6. [Algoritmos y Utilidades](#algoritmos-y-utilidades)
7. [Parámetros Configurables](#parámetros-configurables)



## Introducción

Este proyecto implementa un sistema de Inteligencia Artificial para enemigos basado en un **Árbol de Decisiones** con tres estados principales. Los enemigos son capaces de patrullar, perseguir al jugador cuando lo detectan y retroceder después de atacar.

### Nombre del Sistema
**Sistema de IA Basado en Estados con Detección**

### Objetivo
Proporcionar enemigos con comportamiento dinámico y reactivo que respondan a la presencia del jugador de manera inteligente y predecible, mejorando la experiencia de juego.


## Arquitectura General

La IA se implementa en la clase `Enemy` ubicada en:
- **Header**: `src/objects/Enemy.hpp`
- **Implementación**: `src/objects/Enemy.cpp`

### Componentes Principales

```cpp
class Enemy {
    // Posición y movimiento
    int x, y;                    // Posición en grid (celdas)
    float px, py;                // Posición en píxeles
    int targetX, targetY;        // Objetivo de movimiento
    float speed;                 // Velocidad base
    
    // Estado de la IA
    EnemyState state;            // Estado actual del árbol de decisiones
    float detectionRange;        // Rango de detección (tiles)
    float retreatTimer;          // Temporizador de retroceso
    float retreatDuration;       // Duración del retroceso
};
```


## Estados de la IA

El sistema utiliza un enumerador `EnemyState` que define tres estados mutuamente excluyentes:

### 1. PATROL (Patrulla)
- **Descripción**: Estado por defecto. El enemigo se mueve aleatoriamente por el mapa.
- **Comportamiento**: Movimiento aleatorio a celdas adyacentes transitables.
- **Color de debug**: ROJO (RED)
- **Velocidad**: Base (1.0x)

### 2. CHASE (Persecución)
- **Descripción**: El enemigo ha detectado al jugador y lo persigue activamente.
- **Comportamiento**: Movimiento inteligente hacia la posición del jugador con sistema de prioridades.
- **Color de debug**: NARANJA (ORANGE)
- **Velocidad**: Aumentada (1.3x la base)

### 3. RETREAT (Retroceso)
- **Descripción**: El enemigo se aleja del jugador después de haberlo golpeado.
- **Comportamiento**: Movimiento activo alejándose de la posición del jugador.
- **Color de debug**: PÚRPURA (PURPLE)
- **Velocidad**: Muy aumentada (1.5x la base)
- **Duración**: 3 segundos (configurable)



## Árbol de Decisiones

El sistema de IA se basa en un árbol de decisiones que se evalúa en cada frame durante el método `_updateAI()`.

### Diagrama de Transiciones

```
    ┌──────────┐
    │  PATROL  │ (Estado inicial)
    └────┬─────┘
         │
         │ Jugador detectado Y línea de visión
         ▼
    ┌──────────┐
    │  CHASE   │
    └────┬─────┘
         │
         ├───► Jugador lejos O sin línea de visión ──► Volver a PATROL
         │
         │ onHitPlayer() llamado
         ▼
    ┌──────────┐
    │ RETREAT  │
    └────┬─────┘
         │
         │ Timer agotado (3 segundos)
         ▼
    ┌──────────┐
    │  PATROL  │
    └──────────┘
```

### Condiciones de Transición

#### PATROL → CHASE
```cpp
if (distInTiles <= detectionRange && _hasLineOfSight(map, playerCellX, playerCellY)) {
    state = EnemyState::CHASE;
}
```
- **Condición**: Jugador dentro del rango de detección (6 tiles) Y hay línea de visión directa.

#### CHASE → PATROL
```cpp
if (distInTiles > detectionRange * 1.5f || !_hasLineOfSight(map, playerCellX, playerCellY)) {
    state = EnemyState::PATROL;
}
```
- **Condición**: Jugador fuera del rango extendido (9 tiles) O se pierde la línea de visión.
- **Nota**: El rango extendido (1.5x) evita "parpadeos" en el estado.

#### CHASE → RETREAT
```cpp
void Enemy::onHitPlayer() {
    if (state == EnemyState::CHASE) {
        state = EnemyState::RETREAT;
        retreatTimer = retreatDuration;
    }
}
```
- **Condición**: El enemigo golpea al jugador (método llamado externamente).
- **Nota**: Solo los enemigos que estaban persiguiendo se alejan.

#### RETREAT → PATROL
```cpp
if (retreatTimer <= 0.0f) {
    state = EnemyState::PATROL;
}
```
- **Condición**: Transcurrido el tiempo de retroceso (3 segundos).


## Comportamientos Específicos

### 1. Comportamiento de Patrulla (`_patrolBehavior`)

**Algoritmo**:
1. Espera hasta que termine el cooldown de movimiento
2. Genera un orden aleatorio de direcciones (arriba, abajo, izquierda, derecha)
3. Intenta moverse en cada dirección hasta encontrar una transitable
4. Si ninguna es transitable, permanece quieto

**Características**:
- Movimiento aleatorio utilizando `GetRandomValue()` para barajar direcciones
- Verifica transitabilidad con `map.isWalkableForEnemy()`
- No tiene conocimiento del jugador

### 2. Comportamiento de Persecución (`_chaseBehavior`)

**Algoritmo**:
1. Calcula la celda actual del jugador
2. Determina el delta (diferencia) en X e Y
3. Prioriza direcciones según cuál delta es mayor
4. Intenta moverse en orden de prioridad

**Sistema de Prioridades**:

**Ejemplo**: Si el jugador está más lejos horizontalmente hacia la derecha:
1. **Prioridad 1**: Derecha (acercarse en eje principal)
2. **Prioridad 2**: Vertical hacia el jugador (acercarse en eje secundario)
3. **Prioridad 3**: Vertical opuesta (movimiento lateral)
4. **Prioridad 4**: Izquierda (alejarse - última opción)

**Características**:
- Frecuencia de decisión: cada 0.05 segundos (muy rápido)
- Velocidad aumentada: 1.3x la base
- Movimiento inteligente pero no optimal (no usa pathfinding)
- Puede quedarse atascado en esquinas (comportamiento intencional para dificultad)

### 3. Comportamiento de Retroceso (`_retreatBehavior`)

**Algoritmo**:
1. Invierte la lógica de persecución: calcula dirección opuesta al jugador
2. Prioriza direcciones que alejen del jugador
3. Si no puede alejarse, permanece quieto

**Características**:
- Frecuencia de decisión: cada 0.1 segundos
- Velocidad muy aumentada: 1.5x la base (escape rápido)
- Duración fija: 3 segundos
- Lógica inversa a CHASE (deltaX y deltaY invertidos)


## Algoritmos y Utilidades

### 1. Detección de Línea de Visión (`_hasLineOfSight`)

**Algoritmo**: Bresenham Line Drawing Algorithm

**Propósito**: Determinar si hay una línea directa sin obstáculos entre el enemigo y el jugador.

**Proceso**:
1. Traza una línea desde la celda del enemigo hasta la celda del jugador
2. Verifica cada celda intermedia
3. Si encuentra una pared (`#`), retorna `false`
4. Si llega al jugador sin obstáculos, retorna `true`

**Ventajas**:
- Evita detección a través de paredes
- Comportamiento más realista y justo
- Permite al jugador usar el entorno tácticamente


### 2. Cálculo de Distancia (`_distanceToPlayer`)

**Algoritmo**: Distancia Euclidiana



**Uso**: Determinar si el jugador está dentro del rango de detección.

### 3. Movimiento (`_moveTowardsTarget`)

**Algoritmo**: Interpolación lineal con velocidad variable

**Características**:
- Movimiento suave entre celdas (no teleporte)
- Velocidad adaptada según el estado
- Detección de llegada al objetivo con tolerancia numérica


### 4. Detección de Colisión (`collidesWithPlayer`)

**Algoritmo**: Colisión círculo-rectángulo

**Proceso**:
1. Define el rectángulo del enemigo (bounding box)
2. Encuentra el punto más cercano del rectángulo al centro del círculo (jugador)
3. Verifica si ese punto está dentro del radio del círculo

**Ventajas**:
- Preciso y eficiente
- No requiere librerías externas complejas


## Configuración y valores importantes

Los enemigos tienen algunos parámetros que se pueden ajustar:

**Detección**: Los enemigos pueden ver hasta 6 tiles de distancia (`detectionRange = 6.0f`). Cuando ya están persiguiendo, la distancia se extiende a 9 tiles (1.5x) para que no pierdan al jugador tan fácilmente si se aleja un poco.

**Velocidades**: En PATROL van a velocidad normal (base = tileSize * 1.8). Cuando persiguen (CHASE) van 30% más rápido, y cuando huyen (RETREAT) van 50% más rápido. Esto hace que el juego sea más dinámico.

**Timings**: El retroceso dura 3 segundos. La frecuencia con la que toman decisiones varía: en PATROL depende del cooldown, en CHASE deciden cada 0.05s (muy rápido para perseguir bien), y en RETREAT cada 0.1s.

**Hitbox**: La caja de colisión es un 70% del tamaño de un tile (`bboxW` y `bboxH = tileSize * 0.7`), así que hay un poco de margen y no es súper injusto.


## Consideraciones de Diseño

### Ventajas del Sistema

1. **Predecible**: Los jugadores pueden entender y anticipar el comportamiento
2. **Justo**: La línea de visión evita detecciones injustas
3. **Dinámico**: Tres estados proporcionan variedad táctica
4. **Rendimiento**: Cálculos eficientes, sin pathfinding complejo
5. **Extensible**: Fácil añadir nuevos estados o modificar existentes

### Limitaciones Conocidas

1. **Pathfinding**: No usa A* ni algoritmos avanzados
2. **Sin memoria**: No recuerda dónde vio al jugador por última vez
3. **Comportamiento simple**: Movimiento basado en greedy (codicioso), no óptimo
4. **Sin coordinación**: Los enemigos no colaboran entre sí

### Futuras Mejoras Posibles

- Implementar sistema de sonido (detectar pasos del jugador)
- Añadir memoria corta (buscar en última posición conocida)
- Sistema de alertas entre enemigos
- Pathfinding básico para evitar quedarse atascado
- Estados adicionales (SEARCH, ALERT, etc.)
- Diferentes tipos de enemigos con comportamientos variados


## Referencias

- **Algoritmo de Bresenham**
- **Árboles de Decisión en IA de Videojuegos**: Patrón clásico usado en juegos como Pac-Man, Metal Gear Solid, The Sims
- **State Machines**: Patrón de diseño Finite State Machine (FSM)

