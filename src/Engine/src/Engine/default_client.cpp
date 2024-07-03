#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/colors.hpp>
#include <Engine/default_client.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    DefaultClient::DefaultClient()
    {}

    DefaultClient& DefaultClient::render(class RenderViewport* viewport)
    {
        viewport->rhi_bind();
        float x = (glm::sin(engine_instance->time_seconds()) + 1.f) / 2.f;
        float y = (glm::cos(engine_instance->time_seconds()) + 1.f) / 2.f;
        viewport->rhi_clear_color(Color(x, y, 0.f, 1.f));
        return *this;
    }

    DefaultClient& DefaultClient::update(class RenderViewport* viewport, float dt)
    {
        return *this;
    }

    implement_engine_class_default_init(DefaultClient, 0);
}// namespace Engine
