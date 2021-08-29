#pragma once

namespace k2 {
class EditorLayer;
}

namespace k2::editor {
class IWidget {
public:
    virtual void render(EditorLayer& layer) = 0;
    virtual ~IWidget() = default;
};
}