#pragma once
#include <Core/callback.hpp>
#include <Core/etl/atomic.hpp>
#include <Core/flags.hpp>
#include <Core/pointer.hpp>
#include <Core/structures.hpp>

namespace Engine
{
	struct WindowConfig;
	class Image;

	class ENGINE_EXPORT Window
	{
	public:
		using DestroyCallback = CallBack<void()>;

		CallBacks<void(Window* window)> on_destroy;

	private:
		Pointer<class WindowRenderViewport> m_render_viewport;
		Window* m_parent_window = nullptr;
		Vector<Window*> m_childs;

	protected:
		Atomic<Size2D> m_size;

	public:
		virtual void initialize(const WindowConfig&);
		float_t width();
		float_t height();
		Size2D size();

		virtual Window& width(float_t width);
		virtual Window& height(float_t height);
		virtual Window& size(const Size2D& size);
		virtual String title();
		virtual Window& title(const String& title);
		virtual Point2D position();
		virtual Window& position(const Point2D& position);
		virtual bool resizable();
		virtual Window& resizable(bool value);
		virtual Window& focus();
		virtual bool focused();
		virtual Window& show();
		virtual Window& hide();
		virtual bool is_visible();
		virtual bool is_iconify();
		virtual Window& iconify();
		virtual bool is_restored();
		virtual Window& restore();
		virtual Window& opacity(float value);
		virtual float opacity();
		virtual Window& icon(const Image& image);
		virtual Window& cursor(const Image& image, Vector2i hotspot = {0, 0});
		virtual Window& attribute(const WindowAttribute& attrib, bool value);
		virtual bool attribute(const WindowAttribute& attrib);
		virtual Window& cursor_mode(const CursorMode& mode);
		virtual CursorMode cursor_mode();
		virtual bool support_orientation(Orientation orientation);
		virtual Orientation orientation();
		virtual Identifier id();
		virtual void* native_window();
		virtual size_t monitor_index();

		WindowRenderViewport* render_viewport() const;
		Window* parent_window() const;
		const Vector<Window*>& child_windows() const;

		Window& create_client(const StringView& client_name);

		virtual ~Window();

		friend class WindowManager;
		friend class WindowRenderViewport;
	};
}// namespace Engine
