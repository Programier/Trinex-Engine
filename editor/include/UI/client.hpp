#pragma once
#include <Core/viewport_client.hpp>
#include <UI/menu_bar.hpp>

namespace Trinex::UI
{
	class Context;
	class DockLayoutBuilder;

	class Client : public ViewportClient
	{
		trinex_class(Client, ViewportClient);

	private:
		Context* m_ctx             = nullptr;
		RenderViewport* m_viewport = nullptr;

	public:
		MenuBar menu_bar;

	public:
		static bool register_client(Refl::Class* object_type, Refl::Class* renderer);
		static Client* client_of(Refl::Class* object_type, bool create_if_not_exist = false);

		Client& attach(class RenderViewport* viewport) override;
		Client& deattach(class RenderViewport* viewport) override;
		Client& update(class RenderViewport* viewport, float dt) override;

		virtual Client& setup_dockspace(DockLayoutBuilder& builder);
		virtual Client& select(Object* object);
		virtual Client& update(float dt);

		inline Context* context() const { return m_ctx; }
		inline RenderViewport* viewport() const { return m_viewport; }
	};
}// namespace Trinex::UI
