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
		PathView();
		PathView(const PathView&);
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

		FORCE_INLINE const String& path() const { return m_path; }
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
