#pragma once
#include <type_traits>

namespace Trinex
{
	template<usize segment_size = 4096>
	class Stack
	{
	public:
		static_assert((segment_size & (segment_size - 1)) == 0, "segment_size must be power of two");

	private:
		struct Segment {
			Segment* prev;
			usize used;
			unsigned char* data;

			explicit Segment(Segment* p)
			    : prev(p), used(0), data((unsigned char*) ::operator new(segment_size, std::align_val_t(segment_size)))
			{}

			~Segment() { ::operator delete(data, std::align_val_t(segment_size)); }

			Segment(const Segment&)            = delete;
			Segment& operator=(const Segment&) = delete;
		};

		struct Footer {
			usize object_offset;
			usize previous_used;
		};

	private:
		Segment* m_free    = nullptr;
		Segment* m_current = nullptr;

	private:
		static inline usize align_up(usize value, usize alignment) noexcept { return (value + alignment - 1) & ~(alignment - 1); }

		template<class T>
		static bool layout(usize used, usize& object_offset, usize& footer_offset, usize& end_offset) noexcept
		{
			if (alignof(T) > segment_size)
			{
				return false;
			}

			object_offset = align_up(used, alignof(T));

			if (object_offset > segment_size)
			{
				return false;
			}

			if (sizeof(T) > segment_size - object_offset)
			{
				return false;
			}

			footer_offset = align_up(object_offset + sizeof(T), alignof(Footer));

			if (footer_offset > segment_size)
			{
				return false;
			}

			if (sizeof(Footer) > segment_size - footer_offset)
			{
				return false;
			}

			end_offset = footer_offset + sizeof(Footer);
			return true;
		}

		Footer* top_footer() const { return (Footer*) (m_current->data + m_current->used - sizeof(Footer)); }

		void allocate_segment()
		{
			if (m_free)
			{
				Segment* segment = m_free;
				m_free           = m_free->prev;
				segment->prev    = m_current;
				m_current        = segment;
			}
			else
			{
				m_current = trx_new Segment(m_current);
			}
		}

		void rewind_empty_segments() noexcept
		{
			while (m_current && m_current->used == 0)
			{
				Segment* current = m_current;
				m_current        = current->prev;

				current->prev = m_free;
				m_free        = current;
			}
		}

		void clear_list(Segment* list)
		{
			while (list)
			{
				Segment* prev = list->prev;
				trx_delete list;
				list = prev;
			}
		}

	public:
		Stack() = default;

		~Stack() { clear(); }

		Stack(const Stack&)            = delete;
		Stack& operator=(const Stack&) = delete;

		Stack(Stack&& other) noexcept : m_current(other.m_current), m_free(other.m_free)
		{
			other.m_current = nullptr;
			other.m_free    = nullptr;
		}

		Stack& operator=(Stack&& other) noexcept
		{
			if (this != &other)
			{
				clear();

				m_current       = other.m_current;
				m_free          = other.m_free;
				other.m_current = nullptr;
				other.m_free    = nullptr;
			}

			return *this;
		}

		template<class T, class... Args>
		T* push(const Args&... args)
		{
			static_assert(std::is_trivially_destructible_v<T>, "Stack supports only trivially destructible types");

			usize object_offset = 0;
			usize footer_offset = 0;
			usize end_offset    = 0;

			if (!layout<T>(0, object_offset, footer_offset, end_offset))
			{
				return nullptr;
			}

			if (!m_current || !layout<T>(m_current->used, object_offset, footer_offset, end_offset))
			{
				allocate_segment();
				layout<T>(0, object_offset, footer_offset, end_offset);
			}

			T* object = (T*) (m_current->data + object_offset);
			new (object) T(args...);


			Footer* footer        = (Footer*) (m_current->data + footer_offset);
			footer->object_offset = object_offset;
			footer->previous_used = m_current->used;

			m_current->used = end_offset;

			return object;
		}

		template<class T>
		T* pop()
		{
			static_assert(std::is_trivially_destructible_v<T>, "Stack supports only trivially destructible types");

			rewind_empty_segments();

			if (!m_current || m_current->used == 0)
			{
				return nullptr;
			}

			Footer* footer = top_footer();

			T* object       = (T*) (m_current->data + footer->object_offset);
			m_current->used = footer->previous_used;
			return object;
		}

		void clear() noexcept
		{
			clear_list(m_current);
			clear_list(m_free);

			m_current = nullptr;
			m_free    = nullptr;
		}
	};
};// namespace Trinex
