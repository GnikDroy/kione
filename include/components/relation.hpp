#pragma once

#include <entt/entt.hpp>
#include <queue>
#include <stdexcept>

namespace k2 {

/**
 * @struct RelationComponent
 * @brief A component representing the relationships between entities
 *
 * This struct is used to maintain hierarchical relationships between entities. It tracks the parent
 * of an entity as well as its first, previous, and next siblings in the context of a parent-child relationship.
 *
 * The value of `entt::null` represents no node.
 */
struct RelationComponent {
    /**
     * The number of children this entity has.
     */
    std::size_t children {};

    /**
     * The parent entity in the relationship, or `entt::null` if no parent exists.
     */
    entt::entity parent { entt::null };

    /**
     * The first child entity in the list of siblings, or `entt::null` if no children exist.
     */
    entt::entity first { entt::null };

    /**
     * The previous sibling entity, or `entt::null` if no previous sibling exists.
     *
     */
    entt::entity prev { entt::null };

    /**
     * The next sibling entity, or `entt::null` if no next sibling exists.
     */
    entt::entity next { entt::null };

private:
    /**
     * @brief Attaches a child entity to a parent entity
     * @tparam AttachAtFirst Whether to attach the child at the beginning of the list of children
     * @tparam EntityType The type of entity
     * @param registry The registry containing the entities
     * @param child The child entity to attach
     * @param parent The parent entity to attach the child to
     * @return The relation component of the child entity
     *
     * This function is used to attach a child entity to a parent entity, either at
     * the beginning or end of the list of children. The child is detached from its
     * current ring first.
     *
     * Throws if the attachment would create a cycle.
     *
     */
    template <bool AttachAtFirst, class EntityType>
    static RelationComponent& attach_child(
        entt::basic_registry<EntityType>& registry, EntityType child, EntityType parent) {
        if (is_ancestor_or_self(registry, child, parent)) {
            throw std::runtime_error("RelationComponent: attaching would create a cycle");
        }
        detach(registry, child);
        registry.template get_or_emplace<RelationComponent>(child);
        registry.template get_or_emplace<RelationComponent>(parent);
        auto& parent_relation = registry.template get<RelationComponent>(parent);
        if (parent_relation.children == 0) {
            auto& child_relation = registry.template get<RelationComponent>(child);
            parent_relation.children++;
            parent_relation.first = child;
            child_relation.prev = child;
            child_relation.next = child;
            child_relation.parent = parent;
            return child_relation;
        }
        if constexpr (AttachAtFirst) {
            return attach_before(registry, child, parent_relation.first);
        } else {
            auto last = registry.template get<RelationComponent>(parent_relation.first).prev;
            return attach_after(registry, child, last);
        }
    }

    /**
     * @brief Attaches a sibling entity to another sibling entity
     * @tparam AttachBefore Whether to attach the sibling before the other sibling
     * @tparam EntityType The type of entity
     * @param registry The registry containing the entities
     * @param node The sibling entity to attach
     * @param sibling The sibling entity to attach the other sibling to
     * @return The relation component of the node entity
     *
     * This function is a helper function for attaching a sibling entity to another entity.
     * If sibling is `entt::null`, it will clear the sibling relationships of the node.
     *
     * Throws if the attachment would create a cycle.
     *
     */
    template <bool AttachBefore, class EntityType>
    static RelationComponent& attach_sibling(
        entt::basic_registry<EntityType>& registry, EntityType node, EntityType sibling) {
        if (sibling == entt::null) {
            auto& relation = registry.template get_or_emplace<RelationComponent>(node);
            relation.prev = relation.next = entt::null;
            return relation;
        }

        if (is_ancestor_or_self(registry, node, sibling)) {
            throw std::runtime_error("RelationComponent: attaching would create a cycle");
        }
        detach(registry, node);
        registry.template get_or_emplace<RelationComponent>(node);
        auto& sibling_relation = registry.template get_or_emplace<RelationComponent>(sibling);

        if (sibling_relation.prev == entt::null) {
            sibling_relation.prev = sibling;
            sibling_relation.next = sibling;
        }

        auto& node_relation = registry.template get<RelationComponent>(node);
        node_relation.parent = sibling_relation.parent;

        if (sibling_relation.parent != entt::null) {
            auto& parent_relation = registry.template get<RelationComponent>(sibling_relation.parent);
            if constexpr (AttachBefore) {
                if (parent_relation.first == sibling) {
                    parent_relation.first = node;
                }
            }
            parent_relation.children++;
        }

        if constexpr (AttachBefore) {
            auto prev = sibling_relation.prev;
            auto& prev_relation = registry.template get<RelationComponent>(prev);
            prev_relation.next = node;
            sibling_relation.prev = node;
            node_relation.prev = prev;
            node_relation.next = sibling;
        } else {
            auto next = sibling_relation.next;
            auto& next_relation = registry.template get<RelationComponent>(next);
            next_relation.prev = node;
            sibling_relation.next = node;
            node_relation.prev = sibling;
            node_relation.next = next;
        }

        return node_relation;
    }

public:
    /**
     * @brief Attaches a child entity to a parent entity at the front
     * @tparam EntityType The type of entity
     * @param registry The registry containing the entities
     * @param child The child entity to attach
     * @param parent The parent entity to attach the child to
     * @return The relation component of the child entity
     *
     * This function attaches a child entity to a parent entity at the
     * beginning of the list of children.
     *
     * Throws if the attachment would create a cycle.
     *
     */
    template <class EntityType>
    static RelationComponent& attach_first(
        entt::basic_registry<EntityType>& registry, EntityType child, EntityType parent) {
        return attach_child<true>(registry, child, parent);
    }

    /**
     * @brief Attaches a child entity to a parent entity at the end
     * @tparam EntityType The type of entity
     * @param registry The registry containing the entities
     * @param child The child entity to attach
     * @param parent The parent entity to attach the child to
     * @return The relation component of the child entity
     *
     * This function attaches a child entity to a parent entity at the
     * end of the list of children.
     *
     * Throws if the attachment would create a cycle.
     *
     */
    template <class EntityType>
    static RelationComponent& attach_last(
        entt::basic_registry<EntityType>& registry, EntityType child, EntityType parent) {
        return attach_child<false>(registry, child, parent);
    }

    /**
     * @brief Attaches a sibling entity before another sibling entity
     * @tparam EntityType The type of entity
     * @param registry The registry containing the entities
     * @param node The sibling entity to attach
     * @param sibling The sibling entity to attach the other sibling to
     * @return The relation component of the sibling entity
     *
     * This function attaches a sibling entity to another sibling entity before it.
     *
     * Throws if the attachment would create a cycle.
     *
     */
    template <class EntityType>
    static RelationComponent& attach_before(
        entt::basic_registry<EntityType>& registry, EntityType node, EntityType sibling) {
        return attach_sibling<true>(registry, node, sibling);
    }

    /**
     * @brief Attaches a sibling entity after another sibling entity
     * @tparam EntityType The type of entity
     * @param registry The registry containing the entities
     * @param node The sibling entity to attach
     * @param sibling The sibling entity to attach the other sibling to
     * @return The relation component of the sibling entity
     *
     * This function attaches a sibling entity to another sibling entity after it.
     *
     * Throws if the attachment would create a cycle.
     *
     */
    template <class EntityType>
    static RelationComponent& attach_after(
        entt::basic_registry<EntityType>& registry, EntityType node, EntityType sibling) {
        return attach_sibling<false>(registry, node, sibling);
    }

    /**
     * @brief Whether `ancestor` is `node` itself or appears on its parent chain
     * @tparam EntityType The type of entity
     * @param registry The registry containing the entities
     * @param ancestor The candidate ancestor entity
     * @param node The entity whose parent chain is walked
     * @return Whether `ancestor` is `node` or one of its ancestors
     */
    template <class EntityType>
    static bool is_ancestor_or_self(
        entt::basic_registry<EntityType>& registry, EntityType ancestor, EntityType node) {
        for (auto curr = node; curr != entt::null;) {
            if (curr == ancestor) {
                return true;
            }
            const auto* relation = registry.template try_get<RelationComponent>(curr);
            curr = relation ? relation->parent : EntityType { entt::null };
        }
        return false;
    }

    /**
     * @brief Detaches an entity from its parent and siblings
     * @tparam EntityType The type of entity
     * @param registry The registry containing the entities
     * @param node The entity to detach
     * @return void
     *
     * This function removes an entity from whichever sibling ring it is linked
     * into, including the root ring, and clears its links. Detached entities
     * belong to no ring until they are attached again.
     *
     */
    template <class EntityType> static void detach(entt::basic_registry<EntityType>& registry, EntityType node) {
        auto* relation = registry.template try_get<RelationComponent>(node);
        if (relation == nullptr || relation->parent == entt::null) {
            return;
        }
        auto& parent_relation = registry.template get<RelationComponent>(relation->parent);
        parent_relation.children--;
        if (parent_relation.children == 0) {
            parent_relation.first = entt::null;
        } else if (parent_relation.first == node) {
            parent_relation.first = relation->next;
        }

        auto& next_relation = registry.template get<RelationComponent>(relation->next);
        auto& prev_relation = registry.template get<RelationComponent>(relation->prev);
        next_relation.prev = relation->prev;
        prev_relation.next = relation->next;

        relation->parent = entt::null;
        relation->prev = entt::null;
        relation->next = entt::null;
    }

    /**
     * @brief Gets the list of children of an entity
     * @tparam EntityType The type of entity
     * @param registry The registry containing the entities
     * @param parent The parent entity, or `entt::null` for the root ring
     * @return A vector of pairs containing the entity and its relation component
     *
     * This function gets the list of children of an entity in ring order.
     * If `recursively` is true, it will get the children recursively.
     *
     */
    template <class EntityType>
    static std::vector<std::pair<EntityType, RelationComponent*>> get_children(
        entt::basic_registry<EntityType>& registry, EntityType parent, bool recursively = false) {
        std::vector<std::pair<EntityType, RelationComponent*>> children;
        if (parent == entt::null) {
            return children;
        }
        std::queue<RelationComponent*> rings;
        auto* start = registry.template try_get<RelationComponent>(parent);
        if (start == nullptr) {
            return children;
        }
        rings.push(start);
        while (!rings.empty()) {
            auto& ring = *rings.front();
            rings.pop();
            auto curr = ring.first;
            for (std::size_t i {}; i < ring.children; i++) {
                auto& curr_relation = registry.template get<RelationComponent>(curr);
                children.emplace_back(curr, &curr_relation);
                if (recursively) {
                    rings.push(&curr_relation);
                }
                curr = curr_relation.next;
            }
        }
        return children;
    }
};
}
