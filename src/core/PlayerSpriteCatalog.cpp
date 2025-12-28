#include "PlayerSpriteCatalog.hpp"
#include "ResourceManager.hpp"
#include <algorithm>
#include <filesystem>
#include <iostream>

std::vector<PlayerSpriteSet> DiscoverPlayerSpriteSets(const std::string& baseRelativePath) {
    std::vector<PlayerSpriteSet> sets;
    auto& rm = ResourceManager::Get();

    std::filesystem::path baseAbs = rm.GetAssetPath(baseRelativePath);
    if (!std::filesystem::exists(baseAbs) || !std::filesystem::is_directory(baseAbs)) {
        return sets;
    }

    for (const auto& entry : std::filesystem::directory_iterator(baseAbs)) {
        if (!entry.is_directory()) {
            continue;
        }

        std::string folderName = entry.path().filename().string();
        std::filesystem::path idleAbs = entry.path() / "Idle.png";
        std::filesystem::path walkAbs = entry.path() / "Walk.png";

        PlayerSpriteSet set;
        set.id = folderName;
        set.idlePath = baseRelativePath + "/" + folderName + "/Idle.png";
        set.walkPath = baseRelativePath + "/" + folderName + "/Walk.png";
        set.hasIdle = std::filesystem::exists(idleAbs);
        set.hasWalk = std::filesystem::exists(walkAbs);

        sets.push_back(set);
    }

    std::sort(sets.begin(), sets.end(), [](const PlayerSpriteSet& a, const PlayerSpriteSet& b) {
        return a.id < b.id;
    });

    return sets;
}

std::string ResolveDefaultPlayerSpriteSetId(const std::vector<PlayerSpriteSet>& sets,
                                           const std::string& preferredId) {
    for (const auto& set : sets) {
        if (set.id == preferredId && set.hasIdle && set.hasWalk) {
            return set.id;
        }
    }

    for (const auto& set : sets) {
        if (set.hasIdle && set.hasWalk) {
            return set.id;
        }
    }

    return {};
}

void LogPlayerSpriteSets(const std::vector<PlayerSpriteSet>& sets, const std::string& defaultId) {
    std::cout << "[PlayerSpriteCatalog] Sets encontrados: " << sets.size() << std::endl;
    for (const auto& set : sets) {
        std::cout << "  - " << set.id
                  << " (Idle=" << (set.hasIdle ? "ok" : "missing")
                  << ", Walk=" << (set.hasWalk ? "ok" : "missing") << ")"
                  << std::endl;
    }
    std::cout << "[PlayerSpriteCatalog] Default: "
              << (defaultId.empty() ? "<none>" : defaultId) << std::endl;
}
