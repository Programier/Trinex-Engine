#pragma once
#include <Core/etl/flat_map.hpp>
#include <Core/window.hpp>
#include <Platform/window.hpp>

struct SDL_Window;

namespace Trinex::Platform
{
	class ENGINE_EXPORT SDLWindow final : public Window
	{
	private:
		SDL_Window* m_window = nullptr;

		void destroy_window();
		
	public:
		SDLWindow(SDL_Window* window);
		~SDLWindow();

		u32 monitor() const override;
		u32 id() const override;

		Ref<Window> parent() const override;
		bool parent(Window* window) override;

		Vector2u size() const override;
		bool size(Vector2u size) override;

		Vector2i position() const override;
		bool position(Vector2i position) override;

		const char* title() const override;
		bool title(const char* title) override;

		WindowAttribute attributes(WindowAttribute mask = WindowAttribute::All) const override;
		WindowAttribute attributes(WindowAttribute mask, bool status) override;

		Vector2u minimum_size() const override;
		bool minimum_size(Vector2u size) override;

		Vector2u maximum_size() const override;
		bool maximum_size(Vector2u size) override;

		f32 density() const override;

		bool raise() override;
		bool restore() override;

		f32 opacity() const override;
		bool opacity(f32 opacity) override;
		bool icon(const Image& image) override;

		void* native_handle() const override;
	};

	class ENGINE_EXPORT SDLWindowSystem : public WindowSystem
	{
	private:
		FlatMap<u32, SDLWindow*> m_windows;

	public:
		static SDLWindowSystem* instance();

		Ref<Window> bind(SDL_Window* window);
		SDLWindowSystem& unbind(u32 id);

		Ref<Window> create(const char* title, u32 width, u32 height, WindowAttribute flags) override;
		Ref<Window> grabbed() const override;
		Ref<Window> find(u32 id) const override;
	};
}// namespace Trinex::Platform
