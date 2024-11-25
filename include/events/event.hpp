#pragma once
#include "core/fnv.hpp"

#define HANDLE_EVENT(event_type, dynamic_event, static_event, action)                                                  \
    if (dynamic_event->type == event_type::hash) {                                                                     \
        const auto& static_event = *reinterpret_cast<const event_type*>(dynamic_event);                                \
        (void)static_event;                                                                                            \
        action                                                                                                         \
    }

namespace k2 {
struct Event {
    std::uint64_t type;
    explicit Event(std::uint64_t type);
    virtual ~Event() = default;
};
}