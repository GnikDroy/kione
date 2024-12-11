#pragma once

namespace k2 {
class App {
public:
    App();
    virtual ~App();
    virtual void run() = 0;
};
} // namespace k2
