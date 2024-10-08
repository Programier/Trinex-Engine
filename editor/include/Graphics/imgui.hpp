#pragma once
#include <Core/callback.hpp>
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <Core/object.hpp>
#include <imgui.h>

namespace Engine
{
	class Event;
	class RenderViewport;
	class Window;
}// namespace Engine


namespace Engine
{
	namespace ImGuiBackend_RHI
	{
		extern float rendering_scale_factor;
	}

	namespace ImGuiBackend_Window
	{
		void on_event_recieved(const Event& event);
		void disable_events();
		void enable_events();
	}// namespace ImGuiBackend_Window

	class ImGuiDrawData final
	{
		ImDrawData m_draw_data[2];
		byte m_logic_index  = 0;
		byte m_render_index = 0;

	public:
		ImDrawData* draw_data();
		ImGuiDrawData& release(bool full = false);
		ImGuiDrawData& copy(ImDrawData* draw_data);

		ImGuiDrawData& swap_render_index();
		ImGuiDrawData& swap_logic_index();

		~ImGuiDrawData();
	};

	class ImGuiWidget
	{
	public:
		size_t frame_number = 0;
		bool closable       = true;
		CallBacks<void()> on_close;

		ImGuiWidget();
		delete_copy_constructors(ImGuiWidget);

		virtual void init(RenderViewport* viewport);
		virtual bool render(RenderViewport* viewport) = 0;
		FORCE_INLINE virtual ~ImGuiWidget(){};
	};


	class ImGuiWidgetsList final
	{
		struct Node {
			ImGuiWidget* widget = nullptr;
			Node* next          = nullptr;
			Node* parent        = nullptr;
			const void* id      = nullptr;
		};

		Node* m_root = nullptr;

		ImGuiWidgetsList& push(ImGuiWidget* widget, const void* id = nullptr);
		Node* destroy(Node* node);
		Node* close_window_internal(Node* node);

	public:
		ImGuiWidgetsList() = default;
		delete_copy_constructors(ImGuiWidgetsList);

		template<typename Type, typename... Args>
		Type* create(Args&&... args)
		{
			Type* instance = new Type(std::forward<Args>(args)...);
			push(instance, nullptr);
			return instance;
		}

		template<typename Type, typename... Args>
		Type* create_identified(const void* id, Args&&... args)
		{
			if (id == nullptr)
				return create<Type>(std::forward<Args>(args)...);

			for (Node* node = m_root; node; node = node->next)
			{
				if (node->id == id)
				{
					return reinterpret_cast<Type*>(node->widget);
				}
			}

			Type* instance = new Type(std::forward<Args>(args)...);
			push(instance, id);
			return instance;
		}

		ImGuiWidgetsList& close_widget(class ImGuiWidget* widget);
		ImGuiWidgetsList& close_all_widgets();
		ImGuiWidgetsList& render(class RenderViewport* viewport);
		~ImGuiWidgetsList();
	};


	class ImGuiWindow final : private Object
	{
		declare_class(ImGuiWindow, Object);

	private:
		ImGuiDrawData m_draw_data;
		size_t m_frame;

		ImGuiContext* m_context = nullptr;
		Window* m_window        = nullptr;

		ImGuiWindow& free_resources();

	public:
		ImGuiWidgetsList widgets_list;
		CallBacks<void()> on_destroy;

		bool initialize(Window* window, const Function<void(ImGuiContext*)>& callback);
		bool terminate();

		ImGuiContext* context() const;
		ImDrawData* draw_data();
		ImGuiWindow& new_frame();
		ImGuiWindow& end_frame();
		ImGuiWindow& rhi_render();
		Window* window() const;
		size_t frame_index() const;
		ImGuiWindow& reset_frame_index();
		static ImGuiWindow* current();
		static void make_current(ImGuiWindow*);

		friend class Object;
		friend class Class;
	};
}// namespace Engine


namespace ImGui
{
	bool InputText(const char* label, Engine::String& buffer, ImGuiInputTextFlags flags = 0,
	               ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

	bool InputTextMultiline(const char* label, Engine::String& buffer, const ImVec2& size = ImVec2(0.0f, 0.0f),
	                        ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

	bool InputTextWithHint(const char* label, const char* hint, Engine::String& buffer, ImGuiInputTextFlags flags = 0,
	                       ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

	FORCE_INLINE ImVec2 ImVecFrom(const Engine::Vector2D& vec)
	{
		return {vec.x, vec.y};
	}

	FORCE_INLINE ImVec4 ImVecFrom(const Engine::Vector4D& vec)
	{
		return {vec.x, vec.y, vec.z, vec.w};
	}

	FORCE_INLINE Engine::Vector2D EngineVecFrom(const ImVec2& vec)
	{
		return {vec.x, vec.y};
	}

	FORCE_INLINE Engine::Vector4D EngineVecFrom(const ImVec4& vec)
	{
		return {vec.x, vec.y, vec.z, vec.w};
	}
}// namespace ImGui
