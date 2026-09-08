#pragma once
#include <Core/etl/string.hpp>
#include <Core/etl/vector.hpp>
#include <Core/types/path.hpp>
#include <Platform/enums.hpp>

namespace Trinex
{
	class Window;
}

namespace Trinex::Platform
{
	struct FileDialogFilter {
		String name;
		Vector<String> extensions;
	};

	struct FileDialogDesc {
		String title;
		Path default_path;
		String default_name;
		Vector<FileDialogFilter> filters;
		DialogOption options = DialogOption::FileMustExist;
		Window* parent       = nullptr;
	};

	struct FileDialogResult {
		DialogResult result = DialogResult::Undefined;
		Vector<Path> paths;
	};

	struct MessageDialogDesc {
		String title;
		String message;
		NotificationType type = NotificationType::Message;
		Window* parent        = nullptr;
		DialogOption options  = DialogOption::Modal;
	};

	class ENGINE_EXPORT DialogSystem
	{
	public:
		static DialogSystem* instance();

		virtual ~DialogSystem() = default;

		virtual FileDialogResult open_file(const FileDialogDesc* desc)        = 0;
		virtual FileDialogResult save_file(const FileDialogDesc* desc)        = 0;
		virtual FileDialogResult select_directory(const FileDialogDesc* desc) = 0;
		virtual DialogResult show_message(const MessageDialogDesc* desc)      = 0;
	};
}// namespace Trinex::Platform
