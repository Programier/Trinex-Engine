#pragma once

#include <Core/engine_types.hpp>
#include <Core/etl/map.hpp>
#include <Core/viewport_client.hpp>

namespace Rml
{
	class Element;
	class ElementDocument;
	class Context;
}// namespace Rml

namespace RML = Rml;

namespace Trinex
{
	class RHITexture;
	class RHIContext;
}// namespace Trinex

namespace Trinex::UI
{
	class RMLEngine
	{
	public:
		static void begin_rendering();
		static bool is_rendering();
		static Vector2u render_size();
		static RHITexture* render_target();
		static RHIContext* render_context();
		static void render_texture(RHITexture* texture, const Vector2f& position, const Vector2f& size);
		static void end_rendering();
	};

	class RMLClient : public ViewportClient
	{
		trinex_class(RMLClient, ViewportClient);

	private:
		Map<RML::Element*, class RMLController*> m_controllers;
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

		virtual RMLClient& on_document_load(RML::ElementDocument* document);
		virtual RMLClient& on_document_unload(RML::ElementDocument* document);

		inline RML::Context* context() const { return m_context; }
		inline RenderViewport* viewport() const { return m_viewport; }
		inline RMLController* controller(RML::Element* element) const
		{
			auto it = m_controllers.find(element);
			return it != m_controllers.end() ? it->second : nullptr;
		}

		static RMLClient* from(RML::Element* element);
	};

	class RMLController : public Object
	{
		trinex_class(RMLController, Object);

	public:
		virtual RMLController& attach(RML::Element* element);
		virtual RMLController& update(RML::Element* element);
		virtual RMLController& deattach(RML::Element* element);
	};
}// namespace Trinex::UI
