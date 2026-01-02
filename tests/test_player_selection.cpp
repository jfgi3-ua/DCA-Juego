#include <catch2/catch_test_macros.hpp>
#include <string>
#include "core/PlayerSelection.hpp"

TEST_CASE("PlayerSelection: set/get/clear es determinista", "[player_selection]") {
    // Empezamos en estado limpio para que el test sea reproducible.
    PlayerSelection::ClearSelectedSprite();

    REQUIRE_FALSE(PlayerSelection::HasSelectedSpriteSet());
    REQUIRE_FALSE(PlayerSelection::SelectedHasIdle());
    REQUIRE_FALSE(PlayerSelection::SelectedHasWalk());

    const std::string spriteId = "Knight";
    const std::string idlePath = "sprites/player/Knight/Idle.png";
    const std::string walkPath = "sprites/player/Knight/Walk.png";

    PlayerSelection::SetSelectedSpriteSet(spriteId, idlePath, walkPath, true, false);

    REQUIRE(PlayerSelection::HasSelectedSpriteSet());
    REQUIRE(PlayerSelection::GetSelectedSpriteId() == spriteId);
    REQUIRE(PlayerSelection::GetSelectedIdlePath() == idlePath);
    REQUIRE(PlayerSelection::GetSelectedWalkPath() == walkPath);
    REQUIRE(PlayerSelection::SelectedHasIdle());
    REQUIRE_FALSE(PlayerSelection::SelectedHasWalk());

    PlayerSelection::ClearSelectedSprite();

    REQUIRE_FALSE(PlayerSelection::HasSelectedSpriteSet());
    REQUIRE_FALSE(PlayerSelection::SelectedHasIdle());
    REQUIRE_FALSE(PlayerSelection::SelectedHasWalk());
    REQUIRE(PlayerSelection::GetSelectedSpriteId().empty());
    REQUIRE(PlayerSelection::GetSelectedIdlePath().empty());
    REQUIRE(PlayerSelection::GetSelectedWalkPath().empty());
}
