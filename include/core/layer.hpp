#pragma once

namespace k2 {
    struct Event;

    class Layer {
    public:
        Layer();
        virtual void update(float dt) = 0;
        virtual void render() = 0;
        virtual bool handle_event(const Event*) = 0;
        virtual ~Layer();
    };
}