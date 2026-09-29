#pragma once
#include <Core/callback.hpp>
#include <Core/enums.hpp>
#include <Core/etl/string.hpp>
#include <Core/math/vector.hpp>
#include <Core/pointer.hpp>
#include <Core/ref_counted.hpp>

namespace Trinex
{
	struct WindowConfig;
	class Image;

	struct WindowDesc {
		String title  = "Trinex Engine";
		String client = "";
		Vector2u size = {1280, 720};
		Vector2u pos  = {0, 0};

		WindowAttribute attributes = WindowAttribute::Resizable;
		i16 monitor                = -1;

		static ENGINE_EXPORT const WindowDesc& from_config();
	};

	class ENGINE_EXPORT Window : public RefCounted
	{
	public:
		using DestroyCallback = CallBack<void()>;

		CallBacks<void(Window* window)> on_destroy;

	private:
		Pointer<class RenderViewport> m_render_viewport;
		Window* m_parent_window = nullptr;
		Vector<Window*> m_childs;


	public:
		static Window* create(const char* title, Vector2u size, Window* parent = nullptr);
		static Window* create(const WindowDesc& desc, Window* parent = nullptr);
		static void destroy(Window* window);
		static Window* find(u32 id);
		static Window* main();

	public:
		virtual u32 monitor() const = 0;
		virtual u32 id() const      = 0;

		virtual Vector2u size() const       = 0;
		virtual Window& size(Vector2u size) = 0;

		virtual Vector2i position() const           = 0;
		virtual Window& position(Vector2i position) = 0;

		virtual const char* title() const        = 0;
		virtual Window& title(const char* title) = 0;

		virtual WindowAttribute attributes(WindowAttribute mask = WindowAttribute::All) const = 0;
		virtual WindowAttribute attributes(WindowAttribute mask, bool status)                 = 0;

		virtual Vector2u minimum_size() const       = 0;
		virtual Window& minimum_size(Vector2u size) = 0;

		virtual Vector2u maximum_size() const       = 0;
		virtual Window& maximum_size(Vector2u size) = 0;

		virtual f32 density() const = 0;

		virtual bool raise()   = 0;
		virtual bool restore() = 0;

		virtual f32 opacity() const              = 0;
		virtual Window& opacity(f32 opacity)     = 0;
		virtual Window& icon(const Image& image) = 0;

		virtual void* native_handle() const = 0;

		inline u32 width() const { return size().x; }
		inline u32 height() const { return size().y; }

		inline bool show() { return attributes(WindowAttribute::Hidden, false); }
		inline bool hide() { return attributes(WindowAttribute::Hidden, true); }
		inline bool minimize() { return attributes(WindowAttribute::Minimized, true); }
		inline bool maximize() { return attributes(WindowAttribute::Maximized, true); }

		RenderViewport* render_viewport() const;
		Window* parent_window() const;
		const Vector<Window*>& child_windows() const;

		Window& create_client(const StringView& client_name);

		virtual ~Window();

		friend class RenderViewport;
	};
}// namespace Trinex
