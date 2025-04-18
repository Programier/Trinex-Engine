#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/string.hpp>
#include <Core/etl/vector.hpp>

namespace Engine
{
	class Archive;

	class ENGINE_EXPORT Path final
	{
	private:
		String m_path;

		StringView m_extension;
		StringView m_filename;
		StringView m_stem;
		StringView m_base_path;

		Path& on_path_changed();

	public:
		struct ENGINE_EXPORT Hash {
			size_t operator()(const Path& p) const noexcept;
		};

		static const char separator;
		static const StringView sv_separator;

		Path();
		Path(const Path&);
		Path(const Path&&);
		Path(const StringView& path);
		Path(const char*);
		Path(const String&);

		Path& operator=(const Path&);
		Path& operator=(Path&&);
		Path& operator=(const StringView& path);
		Path& operator=(const String& path);
		Path& operator=(const char* path);
		Path& operator/=(const Path& path);

		Vector<StringView> split_sv() const;
		Vector<String> split() const;
		Path relative(const Path& base) const;

		FORCE_INLINE Path operator/(const Path& path) const
		{
			Path result = *this;
			return result /= path;
		}

		FORCE_INLINE const String& path() const { return m_path; }

		FORCE_INLINE const StringView& extension() const { return m_extension; }

		FORCE_INLINE const StringView& filename() const { return m_filename; }

		FORCE_INLINE const StringView& stem() const { return m_stem; }

		FORCE_INLINE const char* c_str() const { return m_path.c_str(); }

		FORCE_INLINE const String& str() const { return m_path; }

		FORCE_INLINE operator const String&() const { return str(); }

		FORCE_INLINE operator const char*() const { return c_str(); }

		FORCE_INLINE const StringView& base_path() const { return m_base_path; }

		FORCE_INLINE size_t length() const { return m_path.length(); }

		FORCE_INLINE bool empty() const { return length() == 0; }

		FORCE_INLINE bool operator==(const Path& path) const { return m_path == path.m_path; }

		FORCE_INLINE bool operator!=(const Path& path) const { return m_path != path.m_path; }

		FORCE_INLINE Path& operator+=(const StringView& view)
		{
			m_path += view;
			return on_path_changed();
		}

		FORCE_INLINE Path operator+(const StringView& view) const
		{
			Path p = *this;
			return p += view;
		}

		FORCE_INLINE bool operator<(const Path& p) const { return m_path < p.m_path; }

		FORCE_INLINE bool operator>(const Path& p) const { return m_path > p.m_path; }

		FORCE_INLINE bool operator<=(const Path& p) const { return m_path <= p.m_path; }

		FORCE_INLINE bool operator>=(const Path& p) const { return m_path >= p.m_path; }

		FORCE_INLINE bool starts_with(const Path& path) const { return m_path.starts_with(path.m_path); }

		bool serialize(Archive& ar);
	};
}// namespace Engine

namespace std
{
	template<>
	struct hash<Engine::Path> {
		size_t operator()(const Engine::Path& p) const noexcept
		{
			static Engine::Path::Hash h;
			return h(p);
		}
	};
}// namespace std
