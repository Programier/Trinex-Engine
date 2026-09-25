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
		Path relative(PathView base) const;

		PathView extension() const;
		PathView filename() const;
		PathView stem() const;
		PathView base_path() const;

		FORCE_INLINE StringView path() const { return m_path; }
		FORCE_INLINE const char* data() const { return m_path.data(); }
		FORCE_INLINE StringView str() const { return m_path; }
		FORCE_INLINE PathView parent() const { return PathView(base_path()); }
		FORCE_INLINE usize length() const { return m_path.length(); }

		FORCE_INLINE bool empty() const { return length() == 0; }
		FORCE_INLINE bool has_extension() const { return !extension().empty(); }
		FORCE_INLINE bool starts_with(StringView path) const { return m_path.starts_with(path); }

		FORCE_INLINE operator StringView() const { return str(); }
		FORCE_INLINE bool operator==(StringView path) const { return m_path == path; }
		FORCE_INLINE bool operator!=(StringView path) const { return m_path != path; }
		FORCE_INLINE bool operator<(StringView path) const { return m_path < path; }
		FORCE_INLINE bool operator>(StringView path) const { return m_path > path; }
		FORCE_INLINE bool operator<=(StringView path) const { return m_path <= path; }
		FORCE_INLINE bool operator>=(StringView path) const { return m_path >= path; }
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

		PathView split(PathView& remainder, i32 splitter = 1) const;
		Path relative(const Path& base) const;

		FORCE_INLINE PathView extension() const { return view().extension(); }
		FORCE_INLINE PathView filename() const { return view().filename(); }
		FORCE_INLINE PathView stem() const { return view().stem(); }
		FORCE_INLINE PathView base_path() const { return view().base_path(); }

		FORCE_INLINE const char* c_str() const { return m_path.c_str(); }
		FORCE_INLINE const String& str() const { return m_path; }
		FORCE_INLINE PathView parent() const { return PathView(*this).base_path(); }
		FORCE_INLINE usize length() const { return m_path.length(); }

		FORCE_INLINE bool empty() const { return length() == 0; }
		FORCE_INLINE bool has_extension() const { return !extension().empty(); }
		FORCE_INLINE bool starts_with(StringView path) const { return m_path.starts_with(path); }

		FORCE_INLINE operator const String&() const { return str(); }
		FORCE_INLINE operator StringView() const { return str(); }
		FORCE_INLINE operator PathView() const { return PathView(*this); }
		FORCE_INLINE PathView view() const { return PathView(*this); }

		FORCE_INLINE bool operator==(StringView path) const { return m_path == path; }
		FORCE_INLINE bool operator!=(StringView path) const { return m_path != path; }
		FORCE_INLINE bool operator<(StringView path) const { return m_path < path; }
		FORCE_INLINE bool operator>(StringView path) const { return m_path > path; }
		FORCE_INLINE bool operator<=(StringView path) const { return m_path <= path; }
		FORCE_INLINE bool operator>=(StringView path) const { return m_path >= path; }

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
