#pragma once
#include <Core/etl/function.hpp>
#include <Platform/enums.hpp>

namespace Trinex
{
	class Window;
}

namespace Trinex::Platform
{
	struct DialogFileFilter {
		const char* name    = nullptr;
		const char* pattern = nullptr;
	};

	struct DialogResult {
		DialogStatus status;
		const char* const* paths = nullptr;
		usize count              = 0;
		i32 filter               = -1;
	};

	using DialogCallback = Function<void(const DialogResult& result)>;

	class ENGINE_EXPORT DialogSystem
	{
	public:
		static DialogSystem* instance();

		virtual ~DialogSystem() = default;

		virtual DialogSystem& open_file(const DialogCallback& callback, const char* location = nullptr, Window* parent = nullptr,
		                                const DialogFileFilter* filters = nullptr, usize filter_count = 0,
		                                bool multiple = false) = 0;

		virtual DialogSystem& save_file(const DialogCallback& callback, const char* location = nullptr, Window* parent = nullptr,
		                                const DialogFileFilter* filters = nullptr, usize filter_count = 0) = 0;

		virtual DialogSystem& open_directory(const DialogCallback& callback, const char* location = nullptr,
		                                     bool multiple = false, Window* parent = nullptr) = 0;
	};
}// namespace Trinex::Platform
