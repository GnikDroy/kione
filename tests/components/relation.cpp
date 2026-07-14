#include <catch2/catch_test_macros.hpp>

#include "components/relation.hpp"

#include <vector>

namespace {
// Walks the parent's child ring via first/next and checks prev links agree.
std::vector<entt::entity> children_in_order(entt::registry& registry, entt::entity parent) {
    auto& relation = registry.get<k2::RelationComponent>(parent);
    std::vector<entt::entity> result;
    auto curr = relation.first;
    for (std::size_t i = 0; i < relation.children; i++) {
        result.push_back(curr);
        curr = registry.get<k2::RelationComponent>(curr).next;
    }
    return result;
}

bool ring_is_consistent(entt::registry& registry, entt::entity parent) {
    auto& relation = registry.get<k2::RelationComponent>(parent);
    if (relation.children == 0) {
        return relation.first == entt::null;
    }
    auto curr = relation.first;
    for (std::size_t i = 0; i < relation.children; i++) {
        auto& curr_relation = registry.get<k2::RelationComponent>(curr);
        if (curr_relation.parent != parent) {
            return false;
        }
        if (registry.get<k2::RelationComponent>(curr_relation.next).prev != curr) {
            return false;
        }
        if (registry.get<k2::RelationComponent>(curr_relation.prev).next != curr) {
            return false;
        }
        curr = curr_relation.next;
    }
    return curr == relation.first;
}
}

TEST_CASE("attach_last appends children in order", "[relation]") {
    entt::registry registry;
    auto parent = registry.create();
    auto a = registry.create();
    auto b = registry.create();
    auto c = registry.create();
    k2::RelationComponent::attach_last(registry, a, parent);
    k2::RelationComponent::attach_last(registry, b, parent);
    k2::RelationComponent::attach_last(registry, c, parent);

    REQUIRE(children_in_order(registry, parent) == std::vector { a, b, c });
    REQUIRE(ring_is_consistent(registry, parent));
    REQUIRE(registry.get<k2::RelationComponent>(parent).children == 3);
}

TEST_CASE("attach_first prepends children", "[relation]") {
    entt::registry registry;
    auto parent = registry.create();
    auto a = registry.create();
    auto b = registry.create();
    k2::RelationComponent::attach_first(registry, a, parent);
    k2::RelationComponent::attach_first(registry, b, parent);

    REQUIRE(children_in_order(registry, parent) == std::vector { b, a });
    REQUIRE(ring_is_consistent(registry, parent));
}

TEST_CASE("attach_before inserts in the middle and updates first", "[relation]") {
    entt::registry registry;
    auto parent = registry.create();
    auto a = registry.create();
    auto b = registry.create();
    k2::RelationComponent::attach_last(registry, a, parent);
    k2::RelationComponent::attach_last(registry, b, parent);

    auto middle = registry.create();
    k2::RelationComponent::attach_before(registry, middle, b);
    REQUIRE(children_in_order(registry, parent) == std::vector { a, middle, b });
    REQUIRE(ring_is_consistent(registry, parent));

    auto front = registry.create();
    k2::RelationComponent::attach_before(registry, front, a);
    REQUIRE(registry.get<k2::RelationComponent>(parent).first == front);
    REQUIRE(children_in_order(registry, parent) == std::vector { front, a, middle, b });
    REQUIRE(ring_is_consistent(registry, parent));
}

TEST_CASE("attach_after inserts after a sibling", "[relation]") {
    entt::registry registry;
    auto parent = registry.create();
    auto a = registry.create();
    auto b = registry.create();
    k2::RelationComponent::attach_last(registry, a, parent);
    k2::RelationComponent::attach_last(registry, b, parent);

    auto middle = registry.create();
    k2::RelationComponent::attach_after(registry, middle, a);
    REQUIRE(children_in_order(registry, parent) == std::vector { a, middle, b });
    REQUIRE(ring_is_consistent(registry, parent));
}

TEST_CASE("attach_before and attach_after inherit the sibling's parent", "[relation]") {
    entt::registry registry;
    auto parent = registry.create();
    auto a = registry.create();
    k2::RelationComponent::attach_last(registry, a, parent);

    auto before = registry.create();
    auto after = registry.create();
    k2::RelationComponent::attach_before(registry, before, a);
    k2::RelationComponent::attach_after(registry, after, a);

    REQUIRE(registry.get<k2::RelationComponent>(before).parent == parent);
    REQUIRE(registry.get<k2::RelationComponent>(after).parent == parent);
    REQUIRE(registry.get<k2::RelationComponent>(parent).children == 3);
}

TEST_CASE("detach removes a middle child and keeps the ring intact", "[relation]") {
    entt::registry registry;
    auto parent = registry.create();
    auto a = registry.create();
    auto b = registry.create();
    auto c = registry.create();
    k2::RelationComponent::attach_last(registry, a, parent);
    k2::RelationComponent::attach_last(registry, b, parent);
    k2::RelationComponent::attach_last(registry, c, parent);

    k2::RelationComponent::detach(registry, b);

    REQUIRE(children_in_order(registry, parent) == std::vector { a, c });
    REQUIRE(ring_is_consistent(registry, parent));
    REQUIRE((registry.get<k2::RelationComponent>(b).parent == entt::null));
}

TEST_CASE("detach of the first child promotes the next sibling", "[relation]") {
    entt::registry registry;
    auto parent = registry.create();
    auto a = registry.create();
    auto b = registry.create();
    k2::RelationComponent::attach_last(registry, a, parent);
    k2::RelationComponent::attach_last(registry, b, parent);

    k2::RelationComponent::detach(registry, a);

    REQUIRE(registry.get<k2::RelationComponent>(parent).first == b);
    REQUIRE(children_in_order(registry, parent) == std::vector { b });
    REQUIRE(ring_is_consistent(registry, parent));
}

TEST_CASE("detach of the only child empties the parent", "[relation]") {
    entt::registry registry;
    auto parent = registry.create();
    auto child = registry.create();
    k2::RelationComponent::attach_last(registry, child, parent);

    k2::RelationComponent::detach(registry, child);

    auto& relation = registry.get<k2::RelationComponent>(parent);
    REQUIRE(relation.children == 0);
    REQUIRE((relation.first == entt::null));
}

TEST_CASE("detach without a parent is a no-op", "[relation]") {
    entt::registry registry;
    auto entity = registry.create();

    k2::RelationComponent::detach(registry, entity);
    REQUIRE(registry.try_get<k2::RelationComponent>(entity) == nullptr);

    registry.emplace<k2::RelationComponent>(entity);
    k2::RelationComponent::detach(registry, entity);
    REQUIRE((registry.get<k2::RelationComponent>(entity).parent == entt::null));
}

TEST_CASE("an entity can move between parents", "[relation]") {
    entt::registry registry;
    auto old_parent = registry.create();
    auto new_parent = registry.create();
    auto child = registry.create();
    k2::RelationComponent::attach_last(registry, child, old_parent);

    k2::RelationComponent::detach(registry, child);
    k2::RelationComponent::attach_last(registry, child, new_parent);

    REQUIRE(registry.get<k2::RelationComponent>(old_parent).children == 0);
    REQUIRE(children_in_order(registry, new_parent) == std::vector { child });
    REQUIRE(ring_is_consistent(registry, new_parent));
}

TEST_CASE("get_children returns direct children only by default", "[relation]") {
    entt::registry registry;
    auto parent = registry.create();
    auto child = registry.create();
    auto grandchild = registry.create();
    k2::RelationComponent::attach_last(registry, child, parent);
    k2::RelationComponent::attach_last(registry, grandchild, child);

    auto direct = k2::RelationComponent::get_children(registry, parent);
    REQUIRE(direct.size() == 1);
    REQUIRE(direct.front().first == child);

    auto recursive = k2::RelationComponent::get_children(registry, parent, true);
    REQUIRE(recursive.size() == 2);
}

TEST_CASE("get_children of null lists root entities with a relation", "[relation]") {
    entt::registry registry;
    auto root = registry.create();
    auto child = registry.create();
    k2::RelationComponent::attach_last(registry, child, root);

    auto roots = k2::RelationComponent::get_children(registry, entt::entity { entt::null });
    REQUIRE(roots.size() == 1);
    REQUIRE(roots.front().first == root);
}

TEST_CASE("get_children of an entity without a relation is empty", "[relation]") {
    entt::registry registry;
    auto entity = registry.create();
    REQUIRE(k2::RelationComponent::get_children(registry, entity).empty());
}
