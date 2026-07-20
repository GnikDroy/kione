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

namespace {
// A project with a texture, a scene referencing it, and an animation clip referencing it.
std::filesystem::path write_manifest_fixture(const std::string& dir_name) {
    namespace fs = std::filesystem;
    auto dir = fs::temp_directory_path() / dir_name;
    fs::remove_all(dir);
    fs::create_directories(dir);

    std::ofstream { dir / "game.k2project" }
        << "version: 0.0.1\nname: Test\nmain_scene: level\n"
           "assets:\n"
           "  Image:\n    tex: file:///tex.png\n"
           "  Scene:\n    level: file:///level.k2scene\n"
           "  Animation:\n    spin: file:///spin.k2anim\n";
    std::ofstream { dir / "tex.png" } << "";
    std::ofstream { dir / "level.k2scene" }
        << "- Entity: 0\n  SpriteComponent:\n    Texture: tex\n    Color: [1, 1, 1, 1]\n";
    std::ofstream { dir / "spin.k2anim" } << "version: 0.0.1\ntexture: tex\nloop: true\nframes: []\n";
    return dir;
}
}

TEST_CASE("rename_asset moves the manifest key and cascades to references") {
    namespace fs = std::filesystem;
    auto dir = write_manifest_fixture("k2_project_rename_test");

    auto loaded = k2::Project::load(dir / "game.k2project");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->rename_asset("tex", "brick").has_value());

    // Manifest key moved.
    REQUIRE(loaded->assets.count("brick"_fnv1a) == 1);
    REQUIRE(loaded->assets.count("tex"_fnv1a) == 0);

    // Scene and animation references rewritten on disk.
    auto scene = YAML::LoadFile((dir / "level.k2scene").string());
    REQUIRE(scene[0]["SpriteComponent"]["Texture"].as<std::string>() == "brick");
    auto clip = YAML::LoadFile((dir / "spin.k2anim").string());
    REQUIRE(clip["texture"].as<std::string>() == "brick");

    fs::remove_all(dir);
}

TEST_CASE("rename_asset updates main_scene when the scene itself is renamed") {
    namespace fs = std::filesystem;
    auto dir = write_manifest_fixture("k2_project_rename_scene_test");

    auto loaded = k2::Project::load(dir / "game.k2project");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->rename_asset("level", "arena").has_value());
    REQUIRE(loaded->main_scene == "arena");
    REQUIRE(loaded->assets.count("arena"_fnv1a) == 1);

    fs::remove_all(dir);
}

TEST_CASE("rename_asset rejects collisions and unknown names") {
    namespace fs = std::filesystem;
    auto dir = write_manifest_fixture("k2_project_rename_reject_test");

    auto loaded = k2::Project::load(dir / "game.k2project");
    REQUIRE(loaded.has_value());
    REQUIRE_FALSE(loaded->rename_asset("tex", "level").has_value()); // target exists
    REQUIRE_FALSE(loaded->rename_asset("nope", "whatever").has_value()); // source missing
    REQUIRE(loaded->rename_asset("tex", "tex").has_value()); // no-op

    fs::remove_all(dir);
}

TEST_CASE("add_asset registers a file and rejects collisions and escapes") {
    namespace fs = std::filesystem;
    auto dir = write_manifest_fixture("k2_project_add_test");
    std::ofstream { dir / "hero.png" } << "";

    auto loaded = k2::Project::load(dir / "game.k2project");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->add_asset(k2::Asset::Type::Image, "hero", dir / "hero.png").has_value());
    REQUIRE(loaded->assets.count("hero"_fnv1a) == 1);

    REQUIRE_FALSE(loaded->add_asset(k2::Asset::Type::Image, "tex", dir / "hero.png").has_value()); // name taken
    REQUIRE_FALSE(loaded->add_asset(k2::Asset::Type::Image, "outside", "/etc/hosts").has_value()); // escapes root

    fs::remove_all(dir);
}

TEST_CASE("remove_asset drops the key but refuses the main scene") {
    namespace fs = std::filesystem;
    auto dir = write_manifest_fixture("k2_project_remove_test");

    auto loaded = k2::Project::load(dir / "game.k2project");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->remove_asset("tex").has_value());
    REQUIRE(loaded->assets.count("tex"_fnv1a) == 0);

    REQUIRE_FALSE(loaded->remove_asset("level").has_value()); // main scene
    REQUIRE_FALSE(loaded->remove_asset("gone").has_value()); // unknown

    fs::remove_all(dir);
}
