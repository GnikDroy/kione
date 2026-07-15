#pragma once

namespace k2 {
struct Event;

class Layer {
public:
    Layer();
    virtual void begin_frame() { }
    virtual void fixed_update(float dt) { (void) dt; }
    virtual void update(float dt) = 0;
    virtual void render() = 0;
    virtual bool handle_event(const Event*) = 0;
    virtual ~Layer();
};
}
