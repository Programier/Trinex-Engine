#pragma once
#include <Platform/dialogs.hpp>

namespace Trinex::Platform
{
	class SDLDialogSystem : public DialogSystem
	{
	public:
		static SDLDialogSystem* instance();

		SDLDialogSystem& open_file(const DialogCallback& callback, const char* location = nullptr, Window* parent = nullptr,
		                           const DialogFileFilter* filters = nullptr, usize filter_count = 0,
		                           bool multiple = false) override;

		SDLDialogSystem& save_file(const DialogCallback& callback, const char* location = nullptr, Window* parent = nullptr,
		                           const DialogFileFilter* filters = nullptr, usize filter_count = 0) override;

		SDLDialogSystem& open_directory(const DialogCallback& callback, const char* location = nullptr, bool multiple = false,
		                                Window* parent = nullptr) override;
	};
}// namespace Trinex::Platform
