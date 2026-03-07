#pragma once
#include <Core/callback.hpp>
#include <Core/etl/vector.hpp>
#include <Core/pointer.hpp>
#include <Graphics/render_viewport.hpp>
#include <Widgets/imgui_menu_bar.hpp>

struct ImGuiContext;

namespace Trinex
{
	class ImGuiWindow;
	class Window;

	class ImGuiViewportClient : public ViewportClient
	{
		trinex_class(ImGuiViewportClient, ViewportClient);

	public:
		static constexpr u32 s_dock_id = 0x524F4F54;// 'ROOT'

	private:
		ImGuiContext* m_context = nullptr;
		Pointer<ImGuiWindow> m_window;
		WindowRenderViewport* m_viewport = nullptr;

		ImGuiViewportClient& update(class RenderViewport* viewport, float dt) final override;

		void scriptable_update(float dt);
		void scriptable_select(Object* object);

	protected:
		void draw_available_clients_for_opening();

	public:
		ImGuiMenuBar menu_bar;

		template<typename Native>
		struct Scriptable : public Super::Scriptable<Native> {
			Scriptable& update(float dt) override
			{
				ImGuiViewportClient::scriptable_update(dt);
				return *this;
			}

			Scriptable& select(Object* object) override
			{
				ImGuiViewportClient::scriptable_select(object);
				return *this;
			}
		};

		static bool register_client(Refl::Class* object_type, Refl::Class* renderer);
		static ImGuiViewportClient* client_of(Refl::Class* object_type, bool create_if_not_exist = false);

		ImGuiViewportClient();
		~ImGuiViewportClient();

		ImGuiViewportClient& on_bind_viewport(class RenderViewport* viewport) override;
		ImGuiViewportClient& on_unbind_viewport(class RenderViewport* viewport) override;

		inline ImGuiWindow* window() const { return m_window.ptr(); }
		inline WindowRenderViewport* viewport() const { return m_viewport; }
		inline ImGuiContext* context() const { return m_context; }

		virtual ImGuiViewportClient& update(float dt);
		virtual ImGuiViewportClient& select(Object* object);
		virtual u32 build_dock(u32 dock);
	};
}// namespace Trinex
