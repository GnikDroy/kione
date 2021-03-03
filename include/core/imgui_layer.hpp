#include "layer.hpp"

namespace k2 {
    class ImguiLayer: public Layer {
    public:
        ImguiLayer();
        ~ImguiLayer() override;
        void update() override;
    }
}