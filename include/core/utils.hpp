#pragma once

namespace k2 {
    template<class T, class U>
    using result = std::pair<T, U>;

    template <class T>
    concept arithmetic = std::is_arithmetic_v<T>;

    template <class...> constexpr std::false_type always_false{};
}