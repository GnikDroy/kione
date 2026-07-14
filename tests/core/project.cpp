#include <catch2/catch_all.hpp>

#include "core/project.hpp"

#include <filesystem>
#include <fstream>

using namespace k2::literals;

TEST_CASE("Project loads paths and asset bundle") {
    namespace fs = std::filesystem;
    auto dir = fs::temp_directory_path() / "k2_project_test";
    fs::create_directories(dir);

    {
        std::ofstream file { dir / "assets.yaml" };
        file << "version: 0.0.1\nassets:\n  Image:\n    player: file:///textures/player.png\n";
    }
    {
        std::ofstream file { dir / "game.k2project" };
        file << "version: 0.0.1\nname: Test\nassets: assets.yaml\nmain_scene: scene.k2scene\n";
    }

    auto project = k2::Project::load(dir / "game.k2project");
    REQUIRE(project.name == "Test");
    REQUIRE(project.root == fs::absolute(dir));
    REQUIRE(project.main_scene == dir / "scene.k2scene");

    REQUIRE(project.assets.count("player"_fnv1a) == 1);
    auto& [asset_name, asset] = project.assets.at("player"_fnv1a);
    REQUIRE(asset_name == "player");
    REQUIRE(std::string { asset.url }.contains("textures/player.png"));

    fs::remove_all(dir);
}

TEST_CASE("Project rejects malformed files") {
    REQUIRE_THROWS(k2::Project::load(YAML::Load("not a map"), "/tmp"));
}

TEST_CASE("Project save round-trips edited settings") {
    namespace fs = std::filesystem;
    auto dir = fs::temp_directory_path() / "k2_project_save_test";
    fs::create_directories(dir);

    {
        std::ofstream file { dir / "assets.yaml" };
        file << "version: 0.0.1\nassets: {}\n";
    }
    {
        std::ofstream file { dir / "game.k2project" };
        file << "version: 0.0.1\nname: Test\nassets: assets.yaml\nmain_scene: scene.k2scene\n";
    }

    auto project = k2::Project::load(dir / "game.k2project");
    REQUIRE(project.file == fs::absolute(dir / "game.k2project"));
    REQUIRE(project.assets_file == dir / "assets.yaml");

    project.name = "Renamed";
    project.main_scene = project.root / "other.k2scene";
    project.save();

    auto reloaded = k2::Project::load(dir / "game.k2project");
    REQUIRE(reloaded.name == "Renamed");
    REQUIRE(reloaded.main_scene == dir / "other.k2scene");
    REQUIRE(reloaded.assets_file == dir / "assets.yaml");

    fs::remove_all(dir);
}

TEST_CASE("Project without a backing file cannot save") {
    k2::Project project;
    REQUIRE_THROWS(project.save());
}
