#include <catch2/catch_all.hpp>

#include "core/project.hpp"

#include <filesystem>
#include <fstream>

using namespace k2::literals;

TEST_CASE("Project loads settings and the inline asset manifest") {
    namespace fs = std::filesystem;
    auto dir = fs::temp_directory_path() / "k2_project_test";
    fs::create_directories(dir);

    {
        std::ofstream file { dir / "game.k2project" };
        file << "version: 0.0.1\nname: Test\nmain_scene: scene\n"
                "assets:\n  Image:\n    player: file:///textures/player.png\n";
    }

    auto loaded = k2::Project::load(dir / "game.k2project");
    REQUIRE(loaded.has_value());
    auto& project = *loaded;
    REQUIRE(project.name == "Test");
    REQUIRE(project.file == fs::absolute(dir / "game.k2project"));
    REQUIRE(project.root == fs::absolute(dir));
    REQUIRE(project.main_scene == "scene");

    REQUIRE(project.assets.count("player"_fnv1a) == 1);
    auto& [asset_name, asset] = project.assets.at("player"_fnv1a);
    REQUIRE(asset_name == "player");
    REQUIRE(std::string { asset.url }.contains("textures/player.png"));

    fs::remove_all(dir);
}

TEST_CASE("Project manifest recurses into referenced bundles") {
    namespace fs = std::filesystem;
    auto dir = fs::temp_directory_path() / "k2_project_nested_test";
    fs::create_directories(dir / "bundles");

    {
        std::ofstream file { dir / "bundles" / "characters.yaml" };
        file << "version: 0.0.1\nassets:\n  Image:\n    hero: file:///hero.png\n";
    }
    {
        std::ofstream file { dir / "game.k2project" };
        file << "version: 0.0.1\nname: Test\nmain_scene: scene\n"
                "assets:\n  AssetBundle:\n    characters: file:///bundles/characters.yaml\n";
    }

    auto loaded = k2::Project::load(dir / "game.k2project");
    REQUIRE(loaded.has_value());
    auto& project = *loaded;
    REQUIRE(project.assets.count("characters.hero"_fnv1a) == 1);
    auto& [asset_name, asset] = project.assets.at("characters.hero"_fnv1a);
    REQUIRE(asset_name == "characters.hero");
    // Relative urls in a child bundle resolve against the child's directory.
    REQUIRE(std::string { asset.url }.contains("bundles"));

    fs::remove_all(dir);
}

TEST_CASE("Project save round-trips settings and preserves the manifest") {
    namespace fs = std::filesystem;
    auto dir = fs::temp_directory_path() / "k2_project_save_test";
    fs::create_directories(dir);

    {
        std::ofstream file { dir / "game.k2project" };
        file << "version: 0.0.1\nname: Test\nmain_scene: scene\n"
                "assets:\n  Image:\n    player: file:///textures/player.png\n";
    }

    auto loaded = k2::Project::load(dir / "game.k2project");
    REQUIRE(loaded.has_value());
    auto& project = *loaded;
    project.name = "Renamed";
    project.main_scene = "other";
    REQUIRE(project.save().has_value());

    auto reloaded = k2::Project::load(dir / "game.k2project");
    REQUIRE(reloaded.has_value());
    REQUIRE(reloaded->name == "Renamed");
    REQUIRE(reloaded->main_scene == "other");
    REQUIRE(reloaded->assets.count("player"_fnv1a) == 1);

    fs::remove_all(dir);
}

TEST_CASE("Asset name collisions across types fail the load, naming both assets") {
    namespace fs = std::filesystem;
    auto dir = fs::temp_directory_path() / "k2_project_collision_test";
    fs::create_directories(dir);

    {
        std::ofstream file { dir / "game.k2project" };
        file << "version: 0.0.1\nname: Test\nmain_scene: scene\n"
                "assets:\n"
                "  Script:\n    explosion: file:///scripts/explosion.lua\n"
                "  Animation:\n    explosion: file:///animations/explosion.k2anim\n";
    }

    auto loaded = k2::Project::load(dir / "game.k2project");
    REQUIRE_FALSE(loaded.has_value());
    REQUIRE(loaded.error().contains("explosion"));
    REQUIRE(loaded.error().contains("Script"));
    REQUIRE(loaded.error().contains("Animation"));

    fs::remove_all(dir);
}

TEST_CASE("Project rejects malformed files") {
    namespace fs = std::filesystem;
    auto dir = fs::temp_directory_path() / "k2_project_bad_test";
    fs::create_directories(dir);
    {
        std::ofstream file { dir / "bad.k2project" };
        file << "not a map";
    }
    REQUIRE_FALSE(k2::Project::load(dir / "bad.k2project").has_value());
    fs::remove_all(dir);
}

TEST_CASE("Project without a backing file cannot save") {
    k2::Project project;
    REQUIRE_FALSE(project.save().has_value());
}
