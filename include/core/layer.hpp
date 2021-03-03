#pragma once

namespace k2 {
    class Layer {
    public:
        Layer();
        virtual void update() = 0;
        virtual ~Layer();
    };
}