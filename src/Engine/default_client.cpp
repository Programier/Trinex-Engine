#include <Core/base_engine.hpp>
#include <Core/colors.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Engine/default_client.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Graphics/shader_parameters.hpp>
#include <Window/window.hpp>

namespace Engine
{
	DefaultClient::DefaultClient()
	{}

	DefaultClient& DefaultClient::on_bind_viewport(class RenderViewport* viewport)
	{
		return *this;
	}

	DefaultClient& DefaultClient::render(class RenderViewport* viewport)
	{
		return *this;
	}

	DefaultClient& DefaultClient::update(class RenderViewport* viewport, float dt)
	{
		if (engine_instance->frame_index() % 100 == 0)
		{
			info_log("FPS", "%f\n", 1.0 / dt);
		}
		return *this;
	}

	implement_engine_class_default_init(DefaultClient, 0);
}// namespace Engine
