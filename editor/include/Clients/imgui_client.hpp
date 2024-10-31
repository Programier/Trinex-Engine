#pragma once
#include <Core/pointer.hpp>
#include <Graphics/render_viewport.hpp>

namespace Engine
{
	class ImGuiWindow;
	class Window;

	class ImGuiEditorClient : public ViewportClient
	{
		declare_class(ImGuiEditorClient, ViewportClient);

	private:
		Pointer<ImGuiWindow> m_window;
		RenderViewport* m_viewport = nullptr;

		ImGuiEditorClient& update(class RenderViewport* viewport, float dt) final override;

		void scriptable_update(float dt);
		void scriptable_select(Object* object);

	protected:
		void draw_available_clients_for_opening();

	public:
		template<typename Native>
		struct Scriptable : public Super::Scriptable<Native> {
			Scriptable& update(float dt) override
			{
				ImGuiEditorClient::scriptable_update(dt);
				return *this;
			}

			Scriptable& select(Object* object) override
			{
				ImGuiEditorClient::scriptable_select(object);
				return *this;
			}
		};

		static bool register_client(Refl::Class* object_type, Refl::Class* renderer);
		static ImGuiEditorClient* client_of(Refl::Class* object_type, bool create_if_not_exist = false);

		ImGuiEditorClient& on_bind_viewport(class RenderViewport* viewport) override;
		ImGuiEditorClient& on_unbind_viewport(class RenderViewport* viewport) override;
		ImGuiEditorClient& render(class RenderViewport* viewport) override;

		ImGuiWindow* imgui_window() const;
		Window* window() const;
		RenderViewport* viewport() const;


		virtual ImGuiEditorClient& update(float dt);
		virtual ImGuiEditorClient& select(Object* object);
	};
}// namespace Engine
