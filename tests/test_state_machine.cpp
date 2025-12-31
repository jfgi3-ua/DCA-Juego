#include <catch2/catch_test_macros.hpp>
#include <memory>
#include "core/GameState.hpp"
#include "core/StateMachine.hpp"

namespace {
    // Estado dummy para comprobar llamadas a init/pause/resume.
    class DummyState final : public GameState {
        public:
            explicit DummyState(bool* initFlag, bool* pauseFlag, bool* resumeFlag)
                : _initFlag(initFlag), _pauseFlag(pauseFlag), _resumeFlag(resumeFlag) {}

            void init() override {
                if (_initFlag) * _initFlag = true;
            }

            void handleInput() override {}
            void update(float) override {}
            void render() override {}

            void pause() override {
                if (_pauseFlag) *_pauseFlag = true;
            }

            void resume() override {
                if (_resumeFlag) *_resumeFlag = true;
            }

        private:
            bool* _initFlag = nullptr;
            bool* _pauseFlag = nullptr;
            bool* _resumeFlag = nullptr;
    };
}

TEST_CASE("StateMachine: add_state inicializa el nuevo estado", "[state_machine]") {
    StateMachine sm;
    float dt = 0.1f;
    bool initCalled = false;

    sm.add_state(std::make_unique<DummyState>(&initCalled, nullptr, nullptr), false);
    sm.handle_state_changes(dt);

    REQUIRE(initCalled);
    REQUIRE(sm.getCurrentState() != nullptr);
}

TEST_CASE("StateMachine: replace sustituye el estado actual", "[state_machine]") {
    StateMachine sm;
    float dt = 0.1f;
    bool initFirst = false;
    bool initSecond = false;

    sm.add_state(std::make_unique<DummyState>(&initFirst, nullptr, nullptr), false);
    sm.handle_state_changes(dt);
    REQUIRE(initFirst);

    sm.add_state(std::make_unique<DummyState>(&initSecond, nullptr, nullptr), true);
    sm.handle_state_changes(dt);

    REQUIRE(initSecond);
    REQUIRE(sm.getCurrentState() != nullptr);
}

TEST_CASE("StateMachine: remove_state reanuda el estado anterior", "[state_machine]") {
    StateMachine sm;
    float dt = 0.1f;
    bool initFirst = false;
    bool resumeFirst = false;

    sm.add_state(std::make_unique<DummyState>(&initFirst, nullptr, &resumeFirst), false);
    sm.handle_state_changes(dt);
    REQUIRE(initFirst);

    sm.add_state(std::make_unique<DummyState>(nullptr, nullptr, nullptr), false);
    sm.handle_state_changes(dt);

    sm.remove_state(false);
    sm.handle_state_changes(dt);

    REQUIRE(resumeFirst);
}

TEST_CASE("StateMachine: overlay se añade y se elimina correctamente", "[state_machine]") {
    StateMachine sm;
    float dt = 0.1f;
    bool initOverlay = false;

    sm.add_overlay_state(std::make_unique<DummyState>(&initOverlay, nullptr, nullptr));
    sm.handle_state_changes(dt);

    REQUIRE(initOverlay);
    REQUIRE(sm.hasOverlay());

    sm.remove_overlay_state();
    sm.handle_state_changes(dt);

    REQUIRE_FALSE(sm.hasOverlay());
}
