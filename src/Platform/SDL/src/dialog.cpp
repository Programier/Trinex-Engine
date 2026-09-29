#include <SDL3/SDL_dialog.h>
#include <SDLPlatform/dialog.hpp>

namespace Trinex::Platform
{
	static void SDLCALL dialog_callback(void* userdata, const char* const* files, int filter)
	{
		auto* function = static_cast<DialogCallback*>(userdata);

		DialogResult result = {};

		if (!files)
		{
			result.status = DialogStatus::Failed;
		}
		else if (!files[0])
		{
			result.status = DialogStatus::Cancelled;
		}
		else
		{
			result.status = DialogStatus::Accepted;
			result.paths  = files;
			result.filter = filter;

			while (files[result.count]) ++result.count;
		}

		(*function)(result);
		trx_delete function;
	}

	static SDL_Window* sdl_window(Window* window)
	{
		return nullptr;
	}


	SDLDialogSystem* SDLDialogSystem::instance()
	{
		static SDLDialogSystem system;
		return &system;
	}

	SDLDialogSystem& SDLDialogSystem::open_file(const DialogCallback& callback, const char* location, Window* parent,
	                                            const DialogFileFilter* filters, usize filter_count, bool multiple)
	{
		if (!callback)
			return *this;

		SDL_ShowOpenFileDialog(dialog_callback, trx_new DialogCallback(callback), sdl_window(parent),
		                       reinterpret_cast<const SDL_DialogFileFilter*>(filters), static_cast<int>(filter_count), location,
		                       multiple);
		return *this;
	}

	SDLDialogSystem& SDLDialogSystem::save_file(const DialogCallback& callback, const char* location, Window* parent,
	                                            const DialogFileFilter* filters, usize filter_count)
	{
		if (!callback)
			return *this;

		SDL_ShowSaveFileDialog(dialog_callback, trx_new DialogCallback(callback), sdl_window(parent),
		                       reinterpret_cast<const SDL_DialogFileFilter*>(filters), static_cast<int>(filter_count), location);

		return *this;
	}

	SDLDialogSystem& SDLDialogSystem::open_directory(const DialogCallback& callback, const char* location, bool multiple,
	                                                 Window* parent)
	{
		if (!callback)
			return *this;

		SDL_ShowOpenFolderDialog(dialog_callback, trx_new DialogCallback(callback), sdl_window(parent), location, multiple);
		return *this;
	}
}// namespace Trinex::Platform
