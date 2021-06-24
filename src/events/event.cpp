#include "events/event.hpp"

namespace k2 {
Event::Event(std::uint64_t type)
    : type { type } { }
}