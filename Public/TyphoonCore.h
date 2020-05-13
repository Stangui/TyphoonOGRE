#pragma once

#define SAFE_DELETE(x) if (x) { delete x; x = nullptr; }

namespace TyphoonEngine
{
    static constexpr float RENDER_UPDATE_TIME = 1.f/200.f;		// Capped to 200FPS
    static constexpr float LOGIC_UPDATE_TIME = 1.f/60.f;		// Capped to 60FPS
    static constexpr int DEFAULT_WINDOW_WIDTH = 1280;
    static constexpr int DEFAULT_WINDOW_HEIGHT = 720;

    class GraphicsSystem;
    class LogicSystem;
    class GraphicsObjectManager;
    class GraphicsState;
    class LogicState;
    class IBaseState;
}
