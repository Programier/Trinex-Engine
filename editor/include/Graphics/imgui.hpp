#pragma once
#include <Core/callback.hpp>
#include <Core/engine_types.hpp>
#include <Core/math/math.hpp>
#include <Core/object.hpp>
#include <imgui.h>

namespace Engine
{
	struct Event;
	class RenderViewport;
	class Window;
	class RHIContext;
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

	class ImGuiContextLock final
	{
	private:
		ImGuiContext* m_ctx;

	public:
		inline ImGuiContextLock(ImGuiContext* context) : m_ctx(ImGui::GetCurrentContext()) { ImGui::SetCurrentContext(context); }
		inline ~ImGuiContextLock() { ImGui::SetCurrentContext(m_ctx); }
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
			Type* instance = trx_new Type(std::forward<Args>(args)...);
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

			Type* instance = trx_new Type(std::forward<Args>(args)...);
			push(instance, id);
			return instance;
		}

		ImGuiWidgetsList& close_widget(class ImGuiWidget* widget);
		ImGuiWidgetsList& close_all_widgets();
		ImGuiWidgetsList& render(class RenderViewport* viewport);
		~ImGuiWidgetsList();
	};


	namespace Refl
	{
		template<typename T>
		class NativeClass;
	}

	class ImGuiWindow final : private Object
	{
		trinex_declare_class(ImGuiWindow, Object);

	private:
		size_t m_frame;

		ImGuiContext* m_context = nullptr;
		Window* m_window        = nullptr;

		ImGuiWindow& free_resources();

	public:
		ImGuiWidgetsList widgets;
		CallBacks<void()> on_destroy;

		bool initialize(Window* window, ImGuiContext* context);
		bool terminate();

		ImGuiContext* context() const;
		ImGuiWindow& new_frame();
		ImGuiWindow& end_frame();
		Window* window() const;
		size_t frame_index() const;
		static ImGuiWindow* current();
		static void make_current(ImGuiWindow*);

		friend class Object;

		template<typename T>
		friend class Refl::NativeClass;
	};
}// namespace Engine


struct ImGuiWindow;

namespace ImGui
{
	Engine::RHIContext* GetCurrentRHI();

	FORCE_INLINE ImVec4 MakeHoveredColor(ImVec4 color)
	{
		const float factor = 0.2f;

		color.x = Engine::Math::lerp(color.x, 1.f, factor);
		color.y = Engine::Math::lerp(color.y, 1.f, factor);
		color.z = Engine::Math::lerp(color.z, 1.f, factor);
		return color;
	}

	void Paint(ImVec2 size, ImDrawCallback callback, void* userdata = nullptr, size_t userdata_size = 0);

	void TextEllipsis(const char* text, float max_width);

	bool InputText(const char* label, Engine::String& buffer, ImGuiInputTextFlags flags = 0,
	               ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

	bool InputTextMultiline(const char* label, Engine::String& buffer, const ImVec2& size = ImVec2(0.0f, 0.0f),
	                        ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

	bool InputTextWithHint(const char* label, const char* hint, Engine::String& buffer, ImGuiInputTextFlags flags = 0,
	                       ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

	bool ImageButton(ImTextureID user_texture_id, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 0),
	                 const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& bg_col = ImVec4(0, 0, 0, 0),
	                 const ImVec4& tint_col = ImVec4(1, 1, 1, 1));

	float TableGetAutoWidth(const char* name);
	float TableGetAutoWidth(ImGuiID table_id);

	FORCE_INLINE ImVec2 ImVecFrom(const Engine::Vector2f& vec)
	{
		return {vec.x, vec.y};
	}

	FORCE_INLINE ImVec4 ImVecFrom(const Engine::Vector4f& vec)
	{
		return {vec.x, vec.y, vec.z, vec.w};
	}

	FORCE_INLINE Engine::Vector2f EngineVecFrom(const ImVec2& vec)
	{
		return {vec.x, vec.y};
	}

	FORCE_INLINE Engine::Vector4f EngineVecFrom(const ImVec4& vec)
	{
		return {vec.x, vec.y, vec.z, vec.w};
	}
}// namespace ImGui
