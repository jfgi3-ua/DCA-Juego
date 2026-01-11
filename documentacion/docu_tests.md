# Suite de tests con CTest + CI

## Resumen

Incorporación de una suite de tests automatizados integrada con **CTest** y ejecutada en **GitHub Actions**. El objetivo es validar funcionalidades clave del juego con foco en **IO/externos** (filesystem, parseo de mapas, gestión de recursos) y en lógica **headless** (sin GPU/ventana).

**Estado final:**

* CTest habilitado y detectando tests automáticamente.
* Framework de testing integrado: **Catch2**.
* Suite de tests (**≥10** según lo pedido en el enunciado del hito final de la práctica) enfocada en casos límite y lógica reusable.
* CI en GitHub Actions ejecutando **configure + build + ctest** en PRs y `main`.

---

## 1) Integración CMake + CTest

### Cambios realizados

* En el `CMakeLists.txt` raíz:

  * `include(CTest)` y `enable_testing()` colocados **después** de `project(...)`.
* Se añade `add_subdirectory(tests)` para compilar y registrar la suite.
* Se extrae el núcleo del juego a una librería `game_core` (sin `main.cpp`):

  * Permite reutilizar lógica en tests sin levantar ventana / GPU.
* El ejecutable `game` enlaza contra `game_core`.

### Verificación

* Listar tests detectados por CTest:

  ```bash
  ctest --test-dir build -N
  ```

---

## 2) Framework de testing (Catch2)

### Enfoque

Catch2 se integra mediante `FetchContent` en `tests/CMakeLists.txt`.

### Puntos clave

* `Catch2::Catch2WithMain` aporta el `main` del runner de tests.
* `catch_discover_tests(...)` registra tests en CTest automáticamente (sin mantener listas manuales).

---

## 3) Justificación del framework: Catch2 vs Boost.Test y GoogleTest

Se evaluaron **Catch2**, **Boost.Test** y **GoogleTest**. La elección final fue **Catch2** por su equilibrio entre facilidad de integración, bajo coste de adopción y buena compatibilidad con CMake/CTest en un proyecto que no dependía previamente de Boost.

### Por qué Catch2

* **Integración simple en CMake con FetchContent**: incorporación rápida sin introducir un “ecosistema” adicional.
* **Runner incluido** (`Catch2::Catch2WithMain`): no requiere escribir `main` de tests ni boilerplate.
* **Registro automático en CTest** con `catch_discover_tests(...)`: los tests aparecen en CTest sin mantenimiento manual.
* **Excelente DX para tests unitarios**: sintaxis clara, aserciones expresivas y buena legibilidad en PRs.
* **Dependencia acotada**: se añade solo el framework de tests, sin arrastrar librerías externas grandes.
* **Adecuado para “headless + IO”**: encaja con stubs y tests de filesystem/parseo sin imponer requisitos gráficos.

### Por qué no Boost.Test

* **No se usaba Boost previamente**: introducir Boost solo por testing incrementa el peso del stack y la carga de dependencias (instalación, configuración, versionado).
* **Coste de adopción innecesario**: Boost compensa más cuando ya forma parte del proyecto; en este caso añade fricción sin beneficio claro.
* **Complejidad de entorno**: en CI y en entornos heterogéneos, depender de Boost tiende a implicar más ajustes (paquetes del sistema, versiones, toolchains).

### Por qué no GoogleTest

* **Más estructura/boilerplate inicial**: configuración típica más verbosa, especialmente si se quiere un setup “limpio” con integración CTest.
* **Huella conceptual mayor**: es muy completo, pero para el alcance actual (≥10 tests, IO/parseo, headless) Catch2 cubre el caso con menor coste.

**Conclusión:** Catch2 ofrece el mejor ratio **valor/complexidad** para la suite actual, maximizando mantenibilidad y minimizando fricción de integración.

---

## 4) Suite de tests implementada (≥10)

> Objetivo: maximizar cobertura de bordes en parseo/IO y validar componentes reutilizables en modo headless.

### A) IO de mapas (`tests/test_map_io.cpp`)

Casos cubiertos:

* Mapa válido: carga correcta, dimensiones y spawn.
* Archivo inexistente: error.
* Archivo vacío: error.
* Mapa no rectangular: error.
* Mapa sin `P`: error.
* Soporte CRLF: se normaliza `\r`.

**Fixtures:** `tests/fixtures/*.txt`

---

### B) Parseo de mecanismos (`tests/test_map_mechanisms.cpp`)

Casos cubiertos:

* Trigger/target válido: emparejado correcto (DOOR, TRAP, BRIDGE, LEVER).
* Trigger sin target: error.
* Target sin trigger: error.

**Fixtures:** `tests/fixtures/mechanisms_*.txt`

---

### C) ResourceManager en modo headless (`tests/test_resource_manager.cpp`)

Casos cubiertos:

* `GetAssetPath`: resuelve rutas existentes.
* `GetAssetPath`: devuelve el input si la ruta no existe.
* Caché de texturas: evita cargas duplicadas (mismo recurso → misma instancia / no repite carga).

**Nota:** se usan stubs de raylib para evitar dependencia de GPU.

---

### D) PlayerSelection (`tests/test_player_selection.cpp`)

Casos cubiertos:

* Estado global: `set/get/clear`.
* Flags idle/walk: comportamiento determinista (sin efectos residuales entre tests).

---

### E) StateMachine (`tests/test_state_machine.cpp`)

Casos cubiertos:

* `add_state`: inicializa el estado.
* `replace`: sustituye el estado actual.
* `remove_state`: restaura el estado anterior.
* `overlay`: apila y desapila correctamente.

---

## 5) Fixtures, CRLF y modo headless

### ¿Qué son los fixtures?

En testing, los **fixtures** son **datos de prueba controlados** (archivos, entradas o escenarios) preparados para reproducir de forma consistente una situación concreta.

En este proyecto, los fixtures son principalmente **archivos de mapas** con contenido intencionado (válido y con errores) para:

* garantizar que el parseo y la validación se comportan siempre igual,
* evitar depender de archivos reales del juego o del entorno local,
* facilitar la revisión: cada caso de test apunta a un fichero concreto y legible.

**Ubicación:** `tests/fixtures/`

### ¿Por qué se testea CRLF?

El test de **CRLF** valida que el cargador/parseador de mapas es **robusto frente a finales de línea de Windows**.

* Linux/macOS: `LF` (`\n`)
* Windows: `CRLF` (`\r\n`)

Si no se limpia `\r`, pueden aparecer fallos como:

* caracteres “extra” al final de línea,
* errores al detectar tokens (`P`, `E`, `#`, etc.),
* roturas de rectangularidad (cómputo incorrecto de columnas),
* bugs al compartir mapas entre sistemas operativos.

Por eso el test verifica que el parser **normaliza** `\r` (p. ej., eliminándolo) y mantiene el comportamiento correcto.

**Implementación:** se usa un archivo temporal (se crea y borra dentro del test) para no duplicar fixtures solo por el tipo de salto de línea.

### ¿Por qué es necesario el modo headless?

El **modo headless** permite ejecutar tests **sin depender de una ventana gráfica, GPU o contexto OpenGL**, lo cual es crítico para:

* **CI/CD y entornos sin display:** los runners suelen carecer de servidor gráfico y/o aceleración; iniciar render puede fallar aunque la lógica sea correcta.
* **Determinismo:** el render introduce dependencias externas (drivers, timing, foco de ventana) que hacen tests frágiles.
* **Velocidad:** evita inicializaciones costosas y reduce tiempos de ejecución.
* **Aislamiento de lógica:** promueve separar reglas/parseo/estado de la capa de render, mejorando diseño y testabilidad.

#### Cómo se garantiza headless

* No se llaman `InitWindow`, `BeginDrawing` ni `Draw*`.
* Se usan **stubs de raylib** (`tests/raylib_stubs.cpp/.hpp`) para simular carga/descarga de texturas y comprobar caché sin GPU.

---

## 6) CI (GitHub Actions)

Workflow principal: `.github/workflows/ci.yml`

### Triggers

* `pull_request`
* `push` a `main`

### Pasos del pipeline

1. Checkout
2. Instalar dependencias del sistema (X11/GL para builds con raylib, aunque los tests sean headless)
3. Configurar:

   ```bash
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
   ```
4. Compilar:

   ```bash
   cmake --build build
   ```
5. Ejecutar tests:

   ```bash
   ctest --test-dir build --output-on-failure
   ```

---

## 7) Por qué el workflow de empaquetado Debian va separado y solo corre con tags

El workflow de empaquetado Debian (`build-deb.yml`) se mantiene **separado** del CI de validación y se ejecuta **solo con tags** por razones operativas y de calidad:

* **Separación de responsabilidades (validación vs entrega):**

  * `ci.yml` valida cada PR/push (build + tests).
  * `build-deb.yml` genera un artefacto distribuible (paquete), que es un paso de “release”.

* **Evitar coste innecesario en PRs:**

  * Empaquetar suele ser más lento (instalación de toolchain, bundling, metadata, etc.).
  * Ejecutarlo en cada PR incrementa tiempos y consumo de CI sin aportar valor equivalente a la fase de validación.

* **Reducir ruido y falsos negativos:**

  * El packaging introduce variabilidad (entorno, dependencias del sistema, rutas, permisos).
  * Un fallo de empaquetado en una rama de trabajo puede bloquear revisión aunque los tests estén correctos.

* **Reproducibilidad y trazabilidad de releases:**

  * Un **tag** representa un punto estable del repo (versión candidata o release).
  * Ejecutar packaging en tags garantiza que el paquete corresponde a una versión identificable (p. ej. `vX.Y.Z`) y facilita auditoría.

* **Mejor control del ciclo de publicación:**

  * Se empaqueta cuando el equipo decide “esto se entrega”, no en cada iteración.
  * Alinea el pipeline con un flujo de release habitual: merge a `main` → tag → artefactos.

**Resultado:** el CI de PRs permanece rápido y fiable, y el empaquetado se reserva para momentos de publicación/versionado.

---

## 8) Comandos de compilación y tests

### Configurar (generar build)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

* `-S .`: raíz del repo (fuentes)
* `-B build`: directorio de build
* `-DCMAKE_BUILD_TYPE=Debug`: símbolos de depuración y menos optimizaciones

### Compilar

```bash
cmake --build build
```

* `--build build`: compila lo configurado en `build/`

### Listar tests

```bash
ctest --test-dir build -N
```

* `-N`: lista sin ejecutar

### Ejecutar tests

```bash
ctest --test-dir build --output-on-failure
```

* `--output-on-failure`: imprime el log del test cuando falla

---

## 9) Notas técnicas

* `game_core` centraliza el código compartido entre el juego y los tests, evitando dependencias del ejecutable y de `main.cpp`.
* Para evitar problemas con el directorio de trabajo (CWD), los tests usan `TESTS_DIR` definido desde CMake.
* Los stubs de raylib garantizan ejecución headless y permiten validar comportamiento de caché/carga sin GPU.

---

## 10) Implementacion del patron ECS (Entity Component System)

Esta implementacion usa **EnTT** (`vendor/include/entt/entt.hpp`) como motor ECS. El objetivo es desacoplar datos de comportamiento y permitir que el juego trate entidades del mundo como combinaciones de componentes, procesadas por sistemas especializados.

### 10.1) Estructura general

- **Entidades**: identificadores sin logica propia (`entt::entity`), creados y gestionados desde un `entt::registry`.
- **Componentes**: structs simples (datos puros) en `src/ecs/components/`, agrupados por dominio:
  - **World**: `TransformComponent`, `MovementComponent`, `SpriteComponent`, `AnimationComponent`, `ColliderComponent`, `SpikeComponent`, `MechanismComponent`, etc.
  - **Player**: `PlayerInputComponent`, `PlayerStateComponent`, `PlayerStatsComponent`, `PlayerCheatComponent`.
  - **Enemy**: `EnemyAIComponent`.
- **Sistemas**: funciones libres en `src/ecs/systems/` que operan sobre vistas del registro (`registry.view<...>()`).

Punto de entrada central: `src/ecs/Ecs.hpp`, que agrupa componentes y sistemas para su uso en los estados del juego.

### 10.2) Registro y ciclo de juego

El `entt::registry` vive en `src/core/MainGameState.hpp` y se usa en el loop principal:

1. **Creacion de entidades** en `LevelSetupSystem(...)` (`src/ecs/systems/LevelSetupSystem.cpp`).
2. **Update** en `MainGameState::update(...)` (`src/core/MainGameState.cpp`), con el orden:
   - `InputSystem`
   - `EnemyAISystem`
   - `MovementSystem`
   - `AnimationSystem`
   - `SpikeSystem`
   - `InvulnerabilitySystem`
   - `CollisionSystem`
   - `MechanismSystem`
3. **Render** en `_renderMap()` usando `RenderSystem(...)` (y un registro separado para el preview en `SelectPlayerState`).

Este orden es importante: el input decide destinos, el movimiento interpola, la animacion responde a movimiento, las colisiones corrigen estado, y los mecanismos reaccionan al estado del jugador.

### 10.3) Componentes clave y su rol

- `TransformComponent`: posicion y tamano en mundo (base para render y colisiones).
- `MovementComponent`: movimiento por casillas con interpolacion temporal (start/target, progreso, duracion).
- `SpriteComponent` + `GridClipComponent` + `AnimationComponent`: render animado por spritesheet o fijo.
- `ManualSpriteComponent`: recortes manuales para sprites no basados en grid (pinchos, mecanismos).
- `ColliderComponent` + `CollisionType`: hitboxes y categorizacion (Player, Enemy, Spike, Item).
- `MechanismComponent` + `MechanismTriggerComponent` + `MechanismTargetComponent`: enlazan trigger/target por `id` logico.
- `PlayerStateComponent`: invulnerabilidad y ultima casilla valida.
- `EnemyAIComponent`: estado y temporizadores para patrulla, persecucion y retirada.

### 10.4) Sistemas y comportamiento

- **InputSystem**: detecta input, calcula celda destino, actualiza `MovementComponent` y orienta sprite.
- **MovementSystem**: interpola posicion desde `startPos` a `targetPos` segun `deltaTime`.
- **AnimationSystem**: alterna texturas y frames segun si la entidad camina o esta idle.
- **SpikeSystem**: activa/desactiva pinchos en intervalos, sincronizando collider.
- **CollisionSystem**: resuelve colisiones jugador vs peligros/objetos, reduce vidas, invulnerabilidad y recoge items.
- **EnemyAISystem**: cambia entre `Patrol/Chase/Retreat` con line-of-sight y bloqueo por mecanismos.
- **MechanismSystem**: detecta triggers en la celda del jugador y desactiva la pareja trigger/target.
- **LevelSetupSystem**: lee `Map` y genera entidades (jugador, enemigos, pinchos, llaves, mecanismos).
- **RenderSystem**: dibuja entidades segun combinacion de componentes, con reglas de recorte y escalado.

### 10.5) Flujo de datos y dependencias

El ECS no vive aislado: se integra con clases ya existentes del proyecto:

- `Map` (`src/objects/Map.*`) aporta informacion de tiles, walkability y mecanismos.
- `ResourceManager` gestiona texturas cargadas desde assets.
- `PlayerSelection` y `PlayerSpriteCatalog` alimentan los sprites del jugador.

Esto permite que el ECS use los datos de mapa y recursos sin reescribir todo el subsistema de carga.

### 10.6) Ejemplo minimo de entidad

```cpp
auto e = registry.create();
registry.emplace<TransformComponent>(e, Vector2{cx, cy}, Vector2{tile, tile});
registry.emplace<SpriteComponent>(e, tex, Vector2{0,0}, 1.0f);
registry.emplace<ColliderComponent>(e, Rectangle{-w/2, -h/2, w, h}, CollisionType::Item);
registry.emplace<ItemComponent>(e, true);
```

Con esta combinacion, el sistema de colisiones trata la entidad como item recolectable y el render la dibuja.

### 10.7) Migracion de POO a ECS y dificultades

La migracion a ECS se realizo en un punto relativamente **avanzado** del desarrollo. En ese momento ya existia un conjunto de clases orientadas a objetos (por ejemplo en `src/objects/`) y flujos de juego estables. Esto implico:

- **Compatibilidad temporal**: hubo que mantener estructuras antiguas (mapa, recursos, estados) mientras se movia la logica a ECS.
- **Duplicidad de logica**: algunas partes quedaron duplicadas o con "puentes" (por ejemplo, creacion de entidades en `LevelSetupSystem` y validaciones en `MainGameState`).
- **Reasignacion de responsabilidades**: funcionalidad antes embebida en clases ahora vive en sistemas independientes. Esto obligo a redefinir el orden de ejecucion y los puntos de integracion.
- **Ajustes de datos**: atributos antes guardados en objetos (posicion, animacion, colisiones) se repartieron en componentes, requiriendo revisar muchos accesos.
- **Render y recursos**: al migrar tarde, se tuvo que adaptar el render a leer componentes (`SpriteComponent`, `GridClipComponent`, etc.) sin romper las rutas de assets ya existentes.

El resultado es un ECS funcional que convive con el resto del codigo legado, con un flujo de datos mas claro y escalable para ampliar comportamiento por composicion en lugar de herencia.
