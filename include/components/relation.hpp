#pragma once
#include <entt/entt.hpp>

namespace k2 {
struct RelationComponent {
    std::size_t children {};
    entt::entity parent { entt::null };
    entt::entity first { entt::null };
    entt::entity prev { entt::null };
    entt::entity next { entt::null };

private:
    template <bool AttachAtFirst, class EntityType>
    static RelationComponent& attach_child(
        entt::basic_registry<EntityType>& registry, EntityType child, EntityType parent) {
        auto& parent_relation = registry.get_or_emplace<RelationComponent>(parent);
        auto& child_relation = registry.get_or_emplace<RelationComponent>(child);
        if (parent_relation.children == 0) {
            parent_relation.first = child;
            parent_relation.children++;
            child_relation.prev = child_relation.next = child;
            child_relation.parent = parent;
        } else {
            if constexpr (AttachAtFirst) {
                attach_before(registry, child, parent_relation.first);
            } else {
                auto last = registry.get<RelationComponent>(parent_relation.first).prev;
                attach_after(registry, child, last);
            }
        }
        return child_relation;
    }

    template <bool AttachBefore, class EntityType>
    static RelationComponent& attach_sibling(
        entt::basic_registry<EntityType>& registry, EntityType node, EntityType sibling) {
        if (sibling == entt::null) {
            auto& relation = registry.get_or_emplace<RelationComponent>(node);
            relation.prev = relation.next = entt::null;
            return relation;
        }

        auto& sibling_relation = registry.get_or_emplace<RelationComponent>(sibling);
        auto& node_relation = registry.get_or_emplace<RelationComponent>(node);
        node_relation.parent = sibling_relation.parent;

        if constexpr (AttachBefore) {
            auto prev = sibling_relation.prev;
            auto& prev_relation = registry.get<RelationComponent>(prev);
            prev_relation.next = node;
            sibling_relation.prev = node;
            node_relation.prev = prev;
            node_relation.next = sibling;
        } else {
            auto next = sibling_relation.next;
            auto& next_relation = registry.get<RelationComponent>(next);
            next_relation.prev = node;
            sibling_relation.next = node;
            node_relation.prev = sibling;
            node_relation.next = next;
        }

        if (sibling_relation.parent != entt::null) {
            auto& parent_relation = registry.get<RelationComponent>(sibling_relation.parent);
            if constexpr (AttachBefore) {
                if (parent_relation.first == sibling) {
                    parent_relation.first = node;
                }
            }
            parent_relation.children++;
        }
        return node_relation;
    }

public:
    template <class EntityType>
    static RelationComponent& attach_first(
        entt::basic_registry<EntityType>& registry, EntityType child, EntityType parent) {
        return attach_child<true>(registry, child, parent);
    }

    template <class EntityType>
    static RelationComponent& attach_last(
        entt::basic_registry<EntityType>& registry, EntityType child, EntityType parent) {
        return attach_child<false>(registry, child, parent);
    }

    template <class EntityType>
    static RelationComponent& attach_before(
        entt::basic_registry<EntityType>& registry, EntityType node, EntityType sibling) {
        return attach_sibling<true>(registry, node, sibling);
    }

    template <class EntityType>
    static RelationComponent& attach_after(
        entt::basic_registry<EntityType>& registry, EntityType node, EntityType sibling) {
        return attach_sibling<false>(registry, node, sibling);
    }

    template <class EntityType> static void detach(entt::basic_registry<EntityType>& registry, EntityType node) {
        auto* relation = registry.try_get<RelationComponent>(node);
        if (relation && relation->parent != entt::null) {
            auto& parent_relation = registry.get<RelationComponent>(relation->parent);
            if (parent_relation.first == node) {
                parent_relation.first = relation->next;
            }
            parent_relation.children--;
            if (parent_relation.children == 0) {
                parent_relation.first = entt::null;
            }
            relation->parent = entt::null;

            auto& next_relation = registry.get<RelationComponent>(relation->next);
            auto& prev_relation = registry.get<RelationComponent>(relation->prev);
            next_relation.prev = relation->prev;
            prev_relation.next = relation->next;
        }
    }

    template <class EntityType>
    static std::vector<std::pair<EntityType, RelationComponent*>> get_children(
        entt::basic_registry<EntityType>& registry, EntityType parent, bool recursively = false) {
        std::vector<std::pair<EntityType, RelationComponent*>> children;
        if (parent == entt::null) {
            registry.view<RelationComponent>().each([&](auto entity, auto& relation_component) {
                if (relation_component.parent == entt::null || recursively) {
                    children.emplace_back(entity, &relation_component);
                }
            });
        } else {
            std::queue<EntityType> queue;
            queue.push(parent);
            while (!queue.empty()) {
                auto node = queue.front();
                queue.pop();
                auto parent_relation_ptr = registry.try_get<RelationComponent>(node);
                if (parent_relation_ptr) {
                    auto& parent_relation = *parent_relation_ptr;
                    auto curr = parent_relation.first;
                    children.reserve(parent_relation.children);

                    for (std::size_t i {}; i < parent_relation.children; i++) {
                        auto& curr_relation = registry.get<RelationComponent>(curr);
                        children.emplace_back(curr, &curr_relation);
                        if (recursively) {
                            queue.push(curr);
                        }
                        curr = curr_relation.next;
                    }
                }
            }
        }
        return children;
    }
};
}