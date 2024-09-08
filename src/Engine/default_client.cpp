#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/colors.hpp>
#include <Core/logger.hpp>
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
		viewport->rhi_bind();

		auto mat = DefaultResources::Materials::screen;

		static Name name = "screen_texture";
		reinterpret_cast<CombinedImageSamplerMaterialParameterBase*>(mat->find_parameter(name))
		        ->texture_param(DefaultResources::Textures::default_texture);
		mat->apply();
		DefaultResources::Buffers::screen_position->rhi_bind(0);
		rhi->draw(6, 0);

		return *this;
	}

	DefaultClient& DefaultClient::update(class RenderViewport* viewport, float dt)
	{
		return *this;
	}

	implement_engine_class_default_init(DefaultClient, 0);
}// namespace Engine
