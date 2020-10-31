#pragma once
#include "core/window.hpp"

namespace k2
{
    class App
    {
    public:
        App();
        virtual ~App();
        virtual void run();
    };
} // namespace k2