#pragma once
#include <Core/ref_counted.hpp>
#include <Platform/enums.hpp>

namespace Trinex::Platform
{
	struct LibrarySymbol
	{
		void (*address)() = nullptr;

		template<typename T>
		T* as() const
		{
			return reinterpret_cast<T*>(address);
		}

		inline explicit operator bool() const { return address != nullptr; }
	};

	class trinex_api Library : public RefCounted
	{
	public:
		virtual LibrarySymbol find(const char* name) = 0;

		template<typename T>
		T* find_as(const char* name)
		{
			return find(name).as<T>();
		}
	};

	class trinex_api LibraryLoader
	{
	public:
		static LibraryLoader* instance();

		virtual ~LibraryLoader();

		virtual Ref<Library> load(const char* path) = 0;
	};
} // namespace Trinex::Platform
