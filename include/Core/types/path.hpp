#pragma once
#include <Core/etl/string.hpp>

namespace Trinex
{
	class Archive;
	class Path;

	class ENGINE_EXPORT PathView final
	{
	private:
		StringView m_path;

	private:
		PathView(const StringView& path);
		PathView& operator=(const StringView& path);

	public:
		class ComponentIterator
		{
		private:
			StringView m_path;
			usize m_begin = 0;
			usize m_end   = 0;

			inline void find_end()
			{
				m_end = m_begin;
				while (m_end < m_path.size() && m_path[m_end] != '/') ++m_end;
			}

		public:
			ComponentIterator() = default;

			inline ComponentIterator(StringView path, usize begin) : m_path(path), m_begin(begin)
			{
				if (m_begin < m_path.size() && m_begin == 0 && m_path[0] == '/')
					++m_begin;

				if (m_begin >= m_path.size())
				{
					m_begin = m_path.size();
					m_end   = m_path.size();
					return;
				}

				find_end();
			}

			inline PathView operator*() const { return PathView(StringView(m_path.data() + m_begin, m_end - m_begin)); }

			inline ComponentIterator& operator++()
			{
				if (m_end >= m_path.size())
				{
					m_begin = m_path.size();
					m_end   = m_path.size();
					return *this;
				}

				m_begin = m_end + 1;

				if (m_begin >= m_path.size())
				{
					m_begin = m_path.size();
					m_end   = m_path.size();
					return *this;
				}

				find_end();
				return *this;
			}

			inline bool operator==(const ComponentIterator& other) const
			{
				return m_path.data() == other.m_path.data() && m_begin == other.m_begin;
			}

			inline bool operator!=(const ComponentIterator& other) const { return !(*this == other); }
		};

		class ComponentsView
		{
		private:
			StringView m_path;

		public:
			explicit ComponentsView(StringView path) : m_path(path) {}

			inline ComponentIterator begin() const
			{
				if (m_path.empty() || m_path == "/")
					return end();

				return ComponentIterator(m_path, 0);
			}

			inline ComponentIterator end() const { return ComponentIterator(m_path, m_path.size()); }
		};

	public:
		static constexpr bool is_normalized(const char* path, usize size)
		{
			constexpr char sep = '/';

			if (size == 0)
				return true;

			const bool absolute = path[0] == sep;

			// Trailing separator is allowed only for "/".
			if (size > 1)
			{
				if (path[size - 1] == sep)
					return false;
			}

			bool has_regular_component = false;
			usize i                    = 0;

			// Skip root '/'.
			if (absolute)
				i = 1;

			while (i < size)
			{
				// Repeated separator.
				if (path[i] == sep)
					return false;

				const usize begin = i;

				while (i < size && path[i] != sep) ++i;

				const usize length = i - begin;

				// "."
				if (length == 1 && path[begin] == '.')
					return false;

				// ".."
				if (length == 2 && path[begin] == '.' && path[begin + 1] == '.')
				{
					// Absolute paths cannot contain unresolved "..".
					if (absolute)
						return false;

					// Once we have a normal component, ".." could collapse it,
					// so the path isn't normalized.
					if (has_regular_component)
						return false;
				}
				else
				{
					has_regular_component = true;
				}

				// Skip exactly one separator.
				if (i < size)
					++i;
			}

			return true;
		}

		template<usize N>
		    requires(N > 0)
		static constexpr bool is_normalized(const char (&path)[N])
		{
			return is_normalized(path, N - 1);
		}

	public:
		PathView();
		PathView(const PathView&);

		template<usize N>
		    requires(N > 0)
		consteval PathView(const char (&path)[N]) : m_path(path)
		{
			if (!is_normalized(path))
				throw "Path must be normalized";
		}

		explicit PathView(const Path&);

		PathView& operator=(const PathView&) = default;
		PathView& operator=(const Path&);
		Path operator/(PathView path) const;

		PathView split(PathView& remainder, i32 splitter = 1) const;
		Path relative(PathView base);

		PathView extension() const;
		PathView filename() const;
		PathView stem() const;
		PathView parent() const;

		bool starts_with(PathView prefix) const;
		bool ends_with(PathView suffix) const;

		bool is_parent_of(PathView other) const;
		bool is_child_of(PathView other) const;
		bool is_ancestor_of(PathView other) const;

		usize component_count() const;

		PathView root() const;
		PathView first_component() const;
		PathView last_component() const;

		PathView remove_prefix(PathView prefix) const;
		PathView remove_suffix(PathView suffix) const;

		inline bool is_descendant_of(PathView other) const { return other.is_ancestor_of(*this); }
		inline bool is_absolute() const { return !empty() && m_path[0] == '/'; }
		inline bool is_relative() const { return !is_absolute(); }
		inline ComponentsView components() const { return ComponentsView(m_path); }
		inline StringView path() const { return m_path; }
		inline const char* data() const { return m_path.data(); }
		inline StringView str() const { return m_path; }
		inline usize length() const { return m_path.length(); }
		inline usize size() const { return m_path.size(); }

		inline bool empty() const { return length() == 0; }

		inline operator StringView() const { return str(); }
		inline bool operator==(StringView path) const { return m_path == path; }
		inline bool operator!=(StringView path) const { return m_path != path; }
		inline bool operator<(StringView path) const { return m_path < path; }
		inline bool operator>(StringView path) const { return m_path > path; }
		inline bool operator<=(StringView path) const { return m_path <= path; }
		inline bool operator>=(StringView path) const { return m_path >= path; }
		inline char operator[](u32 index) const { return m_path[index]; }
	};

	class ENGINE_EXPORT Path final
	{
	private:
		String m_path;
		Path& on_path_changed();

	public:
		struct ENGINE_EXPORT Hash {
			usize operator()(const Path& p) const noexcept;
		};

		static const char separator;
		static const StringView sv_separator;

		Path();
		Path(const Path&);
		Path(Path&&);
		Path(const PathView& path);
		Path(const StringView& path);
		Path(const char*);
		Path(const String&);

		Path& operator=(const Path&);
		Path& operator=(Path&&);
		Path& operator=(const PathView& path);
		Path& operator=(const StringView& path);
		Path& operator=(const String& path);
		Path& operator=(const char* path);
		Path& operator/=(const Path& path);
		Path& operator/=(PathView path);
		Path& operator+=(const StringView& view)
		{
			m_path += view;
			return on_path_changed();
		}

		Path operator/(const Path& path) const
		{
			Path result = *this;
			return result /= path;
		}

		Path operator+(const StringView& view) const
		{
			Path p = *this;
			return p += view;
		}

		inline PathView split(PathView& remainder, i32 splitter = 1) const { return view().split(remainder, splitter); }
		inline Path relative(PathView base) const { return view().relative(base); }

		inline PathView extension() const { return view().extension(); }
		inline PathView filename() const { return view().filename(); }
		inline PathView stem() const { return view().stem(); }
		inline PathView parent() const { return view().parent(); }

		inline bool is_parent_of(PathView other) const { return view().is_parent_of(other); }
		inline bool is_child_of(PathView other) const { return view().is_child_of(other); }
		inline bool is_ancestor_of(PathView other) const { return view().is_ancestor_of(other); }
		inline bool is_descendant_of(PathView other) const { return view().is_descendant_of(other); }
		inline bool is_absolute() const { return view().is_absolute(); }
		inline bool is_relative() const { return view().is_relative(); }

		inline usize component_count() const { return view().component_count(); }
		inline PathView::ComponentsView components() const { return view().components(); }
		inline StringView path() const { return m_path; }
		inline const char* data() const { return m_path.data(); }

		inline PathView root() const { return view().root(); }
		inline PathView first_component() const { return view().first_component(); }
		inline PathView last_component() const { return view().last_component(); }

		inline PathView remove_prefix(PathView prefix) const { return view().remove_prefix(prefix); }
		inline PathView remove_suffix(PathView suffix) const { return view().remove_suffix(suffix); }

		inline const char* c_str() const { return m_path.c_str(); }
		inline const String& str() const { return m_path; }
		inline usize length() const { return m_path.length(); }
		inline usize size() const { return m_path.size(); }

		inline bool empty() const { return length() == 0; }
		inline bool starts_with(StringView path) const { return m_path.starts_with(path); }
		inline bool ends_with(StringView path) const { return m_path.ends_with(path); }

		inline operator const String&() const { return str(); }
		inline operator StringView() const { return str(); }
		inline operator PathView() const { return PathView(*this); }
		inline PathView view() const { return PathView(*this); }

		inline bool operator==(StringView path) const { return m_path == path; }
		inline bool operator!=(StringView path) const { return m_path != path; }
		inline bool operator<(StringView path) const { return m_path < path; }
		inline bool operator>(StringView path) const { return m_path > path; }
		inline bool operator<=(StringView path) const { return m_path <= path; }
		inline bool operator>=(StringView path) const { return m_path >= path; }
		inline char operator[](u32 index) const { return m_path[index]; }

		bool serialize(Archive& ar);
	};
}// namespace Trinex

namespace std
{
	template<>
	struct hash<Trinex::Path> {
		size_t operator()(const Trinex::Path& p) const noexcept
		{
			static Trinex::Path::Hash h;
			return h(p);
		}
	};
}// namespace std
