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
		material = Object::load_object("Example::Example")->instance_cast<Material>();
		vb       = Object::new_instance<VB>();
		cb       = Object::new_instance<ColorVertexBuffer>();

		vb->buffer = {{-1, -1}, {-1, 1}, {1, 1}};
		cb->buffer = {{255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255}};

		vb->init_resource();
		cb->init_resource();

		return *this;
	}

	DefaultClient& DefaultClient::render(class RenderViewport* viewport)
	{
		viewport->rhi_bind();

		viewport->rhi_clear_color(Color(0.0, 0.0, 0.0, 1.0));
		material->apply();
		vb->rhi_bind(0);
		cb->rhi_bind(1);

		rhi->draw(3, 0);

		return *this;
	}

	DefaultClient& DefaultClient::update(class RenderViewport* viewport, float dt)
	{
		return *this;
	}

	implement_engine_class_default_init(DefaultClient, 0);
}// namespace Engine
