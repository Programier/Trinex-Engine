#pragma once
#include <Core/etl/function.hpp>
#include <Core/etl/string.hpp>
#include <Core/etl/vector.hpp>
#include <Core/types/path.hpp>
#include <Platform/enums.hpp>

namespace Trinex::Platform
{
	class ENGINE_EXPORT Clipboard
	{
	public:
		static Clipboard* instance();
		virtual ~Clipboard() = default;

		virtual usize mime_types() const                   = 0;
		virtual const char* mime_type(usize idx = 0) const = 0;

		virtual bool clear()                                                                           = 0;
		virtual bool load(const FunctionRef<void(const u8*, usize)>& func, const char* mime = nullptr) = 0;
		virtual bool store(const void* data, usize size, const char* mime)                             = 0;
	};
}// namespace Trinex::Platform
