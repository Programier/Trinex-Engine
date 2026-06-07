#include <RHI/handles.hpp>
#include <RmlUi/Core/ElementInstancer.h>
#include <RmlUi/Core/Factory.h>
#include <UI/controllers/canvas.hpp>
#include <UI/elements/canvas.hpp>
#include <UI/rml.hpp>
#include <algorithm>

namespace Trinex::UI
{
	namespace
	{
		static void fit_canvas_frame(const RMLCanvasFrame& frame, Vector2f& position, Vector2f& size)
		{
			if (frame.texture == nullptr || size.x <= 0.f || size.y <= 0.f)
				return;

			Vector3u texture_size_3d = frame.texture->size();
			if (texture_size_3d.x == 0 || texture_size_3d.y == 0)
				return;

			const Vector2f texture_size = {static_cast<float>(texture_size_3d.x), static_cast<float>(texture_size_3d.y)};

			switch (frame.fit)
			{
				case RMLCanvasFit::Stretch: return;

				case RMLCanvasFit::Center:
				{
					const Vector2f clamped = {std::min(size.x, texture_size.x), std::min(size.y, texture_size.y)};
					position += (size - clamped) * 0.5f;
					size = clamped;
					return;
				}

				case RMLCanvasFit::Contain:
				{
					const float scale     = std::min(size.x / texture_size.x, size.y / texture_size.y);
					const Vector2f fitted = texture_size * scale;
					position += (size - fitted) * 0.5f;
					size = fitted;
					return;
				}
			}
		}
	}// namespace

	trinex_on_pre_init({.after = {"RML"}})
	{
		static RML::ElementInstancerGeneric<RMLCanvasElement> instancer;
		RML::Factory::RegisterElementInstancer("canvas", &instancer);
	}

	void RMLCanvasElement::OnRender()
	{
		RMLClient* client = RMLClient::from(this);

		if (client == nullptr)
			return;

		auto controller = Object::instance_cast<RMLCanvasController>(client->controller(this));

		if (controller == nullptr)
			return;

		const Rml::Vector2f position = GetAbsoluteOffset(Rml::BoxArea::Content);
		const Rml::Vector2f size     = GetBox().GetSize(Rml::BoxArea::Content);

		if (size.x <= 0.f || size.y <= 0.f)
			return;

		RMLCanvasRenderArgs args = {
		        .element  = this,
		        .position = {position.x, position.y},
		        .size     = {size.x, size.y},
		};

		RMLCanvasFrame frame = controller->render(this, args);

		if (frame.texture == nullptr)
			return;

		Vector2f fitted_position = args.position;
		Vector2f fitted_size     = args.size;
		fit_canvas_frame(frame, fitted_position, fitted_size);
		RMLEngine::render_texture(frame.texture, fitted_position, fitted_size);
	}
}// namespace Trinex::UI
