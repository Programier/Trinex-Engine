#pragma once
#include <Core/callback.hpp>
#include <Core/enums.hpp>
#include <Core/etl/atomic.hpp>
#include <Core/etl/string.hpp>
#include <Core/math/vector.hpp>
#include <Core/pointer.hpp>

namespace Trinex
{
	struct WindowConfig;
	class Image;

	class ENGINE_EXPORT Window
	{
	public:
		using DestroyCallback = CallBack<void()>;

		CallBacks<void(Window* window)> on_destroy;

	private:
		Pointer<class RenderViewport> m_render_viewport;
		Window* m_parent_window = nullptr;
		Vector<Window*> m_childs;

	protected:
		Atomic<Vector2u> m_size;

	public:
		virtual void initialize(const WindowConfig&);
		f32 width();
		f32 height();
		Vector2u size();

		virtual Window& width(f32 width);
		virtual Window& height(f32 height);
		virtual Window& size(const Vector2u& size);
		virtual String title();
		virtual Window& title(const String& title);
		virtual Vector2u position();
		virtual Window& position(const Vector2u& position);
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
		virtual usize monitor_index();

		RenderViewport* render_viewport() const;
		Window* parent_window() const;
		const Vector<Window*>& child_windows() const;

		Window& create_client(const StringView& client_name);

		virtual ~Window();

		friend class WindowManager;
		friend class RenderViewport;
	};
}// namespace Trinex
