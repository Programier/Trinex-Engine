#pragma once

namespace Trinex
{
	template<usize size, usize align = 16>
	struct alignas(align) Storage {
		static_assert(size > 0);
		static_assert((align & (align - 1)) == 0, "alignment must be power of two");

		alignas(align) u8 data[size];

	private:
		static constexpr usize align_up(usize offset, usize alignment) { return (offset + alignment - 1) & ~(alignment - 1); }

		template<typename T, usize offset>
		static consteval void validate()
		{
			static_assert(offset + sizeof(T) <= size, "Offset out of bounds");
			static_assert(align % alignof(T) == 0, "Storage alignment is not compatible with T");
			static_assert(offset % alignof(T) == 0, "Offset is not properly aligned for T");
		}

	public:
		template<typename T, usize offset = 0>
		T* ptr()
		{
			validate<T, offset>();
			return trinex_launder(reinterpret_cast<T*>(data + offset));
		}

		template<typename T, usize offset = 0>
		const T* ptr() const
		{
			validate<T, offset>();
			return trinex_launder(reinterpret_cast<const T*>(data + offset));
		}

		template<typename T, usize offset = 0>
		T& as()
		{
			return *ptr<T, offset>();
		}

		template<typename T, usize offset = 0>
		const T& as() const
		{
			return *ptr<T, offset>();
		}

		template<typename T, usize offset = 0, typename... Args>
		T& construct(Args&&... args)
		{
			validate<T, offset>();
			void* ptr = data + offset;

			return *::new (ptr) T(static_cast<Args&&>(args)...);
		}

		template<typename T, usize offset = 0>
		void destroy()
		{
			as<T, offset>().~T();
		}
	};
}// namespace Trinex
