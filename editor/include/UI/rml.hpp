#pragma once

#include <Core/engine_types.hpp>
#include <Core/viewport_client.hpp>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/SystemInterface.h>

namespace RML = Rml;

namespace Trinex::UI
{
	class RMLClient : public ViewportClient
	{
		trinex_class(RMLClient, ViewportClient);

	private:
		RML::Context* m_context    = nullptr;
		RenderViewport* m_viewport = nullptr;

	public:
		RMLClient& attach(class RenderViewport* viewport) override;
		RMLClient& deattach(class RenderViewport* viewport) override;
		RMLClient& update(class RenderViewport* viewport, float dt) override;

		EventDispatchResult on_quit(RoutedEvent& event) override;
		EventDispatchResult on_window_event(WindowEvent& event) override;
		EventDispatchResult on_key_event(KeyEvent& event) override;
		EventDispatchResult on_text_input_event(TextInputEvent& event) override;
		EventDispatchResult on_pointer_event(PointerEvent& event) override;

		inline RML::Context* context() const { return m_context; }
		inline RenderViewport* viewport() const { return m_viewport; }
	};
}// namespace Trinex::UI
