#include <Core/archive.hpp>
#include <Core/etl/hash.hpp>
#include <Core/math/math.hpp>
#include <Core/string_functions.hpp>
#include <Core/types/path.hpp>

namespace Trinex
{
	static_assert(PathView::is_normalized(""));
	static_assert(PathView::is_normalized("/"));
	static_assert(PathView::is_normalized("foo"));
	static_assert(PathView::is_normalized("foo/bar"));

	static_assert(PathView::is_normalized("/"));
	static_assert(PathView::is_normalized("[foo]:"));
	static_assert(PathView::is_normalized("[foo]:/bar"));

	static_assert(PathView::is_normalized(".."));
	static_assert(PathView::is_normalized("../foo"));
	static_assert(PathView::is_normalized("../../foo"));
	static_assert(PathView::is_normalized("../../foo/bar"));

	static_assert(!PathView::is_normalized("./foo"));
	static_assert(!PathView::is_normalized("foo/."));

	static_assert(!PathView::is_normalized("foo/.."));
	static_assert(!PathView::is_normalized("foo/../bar"));
	static_assert(!PathView::is_normalized("../foo/.."));

	static_assert(!PathView::is_normalized("/.."));
	static_assert(!PathView::is_normalized("/foo/.."));
	static_assert(!PathView::is_normalized("/foo/../bar"));

	static_assert(!PathView::is_normalized("foo//bar"));
	static_assert(!PathView::is_normalized("/foo//bar"));
	static_assert(!PathView::is_normalized("foo/"));

	const char Path::separator          = '/';
	const StringView Path::sv_separator = "/";


	usize Path::Hash::operator()(const Path& p) const noexcept
	{
		static Trinex::Hash<String> hasher;
		return hasher(p.m_path);
	}

	static void simplify_path(String& path)
	{
		if (path.empty())
			return;

		constexpr char sep = Path::separator;

		char* data       = path.data();
		const usize size = path.size();

		usize read  = 0;
		usize write = 0;

		const bool absolute = data[0] == sep;

		// Preserve root.
		if (absolute)
		{
			data[write++] = sep;

			while (read < size && data[read] == sep) ++read;
		}

		auto append_component = [&](usize begin, usize length) {
			if (write != 0 && data[write - 1] != sep)
				data[write++] = sep;

			// Most already-normalized paths hit write == begin,
			// so no copy is performed.
			if (write != begin)
				std::memmove(data + write, data + begin, length);

			write += length;
		};

		auto previous_component_begin = [&]() {
			const usize limit = absolute ? 1 : 0;

			usize begin = write;

			while (begin > limit && data[begin - 1] != sep) --begin;

			return begin;
		};

		auto pop_component = [&]() {
			const usize limit = absolute ? 1 : 0;
			usize begin       = previous_component_begin();

			write = begin;

			// Remove separator before the component.
			if (write > limit && data[write - 1] == sep)
				--write;
		};

		while (read < size)
		{
			// Skip repeated separators.
			while (read < size && data[read] == sep) ++read;

			if (read == size)
				break;

			const usize begin = read;

			while (read < size && data[read] != sep) ++read;

			const usize length = read - begin;

			// "."
			if (length == 1 && data[begin] == '.')
				continue;

			// ".."
			if (length == 2 && data[begin] == '.' && data[begin + 1] == '.')
			{
				const usize limit = absolute ? 1 : 0;

				if (write > limit)
				{
					const usize prev_begin = previous_component_begin();
					const usize prev_size  = write - prev_begin;

					// For relative paths:
					// "../../foo" must preserve leading "..".
					const bool previous_is_parent = prev_size == 2 && data[prev_begin] == '.' && data[prev_begin + 1] == '.';

					if (!previous_is_parent)
					{
						pop_component();
						continue;
					}
				}

				if (!absolute)
					append_component(begin, length);

				// For absolute paths we clamp ".." at root:
				// "/../../foo" -> "/foo"
				continue;
			}

			append_component(begin, length);
		}

		// Keep "/" intact, remove trailing separator everywhere else.
		if (write > 1 && data[write - 1] == sep)
			--write;

		path.resize(write);
	}

	PathView::PathView() : m_path() {}

	PathView::PathView(const PathView&) = default;

	PathView::PathView(const Path& path) : m_path(path.str()) {}

	PathView::PathView(const StringView& path) : m_path(path) {}

	PathView& PathView::operator=(const Path& path)
	{
		m_path = path.str();
		return *this;
	}

	PathView& PathView::operator=(const StringView& path)
	{
		m_path = path;
		return *this;
	}

	PathView PathView::split(PathView& remainder, i32 splitter) const
	{
		if (empty())
		{
			remainder = PathView();
			return {};
		}

		if (splitter == 0)
		{
			remainder = *this;
			return {};
		}

		if (splitter < 0)
		{
			usize current = m_path.length();

			while (splitter++ < 0)
			{
				if (current == 0)
				{
					remainder = *this;
					return {};
				}

				current = m_path.rfind(Path::separator, current - 1);

				if (current == StringView::npos)
				{
					PathView result = *this;
					remainder       = *this;
					return result;
				}
			}

			PathView result = m_path.substr(0, current);
			remainder       = m_path.substr(current + 1);
			return result;
		}
		else
		{
			usize index   = 0;
			usize current = StringView::npos;

			while (splitter-- > 0)
			{
				current = m_path.find(Path::separator, index);

				if (current == StringView::npos)
				{
					PathView result = *this;
					remainder       = PathView();
					return result;
				}

				index = current + 1;
			}

			PathView result = m_path.substr(0, current);
			remainder       = m_path.substr(current + 1);
			return result;
		}
	}

	Path PathView::relative(PathView base)
	{
		// Absolute and relative paths belong to different namespaces.
		if (is_absolute() != base.is_absolute())
			return {};

		auto path_it  = components().begin();
		auto path_end = components().end();

		auto base_it  = base.components().begin();
		auto base_end = base.components().end();

		// Find common prefix.
		while (path_it != path_end && base_it != base_end && *path_it == *base_it)
		{
			++path_it;
			++base_it;
		}

		Path result;

		// For every remaining component in base,
		// we need to go one directory up.
		for (; base_it != base_end; ++base_it) result /= PathView("..");

		// Then append the unmatched part of this path.
		for (; path_it != path_end; ++path_it) result /= *path_it;

		return result;
	}

	Path PathView::operator/(PathView path) const
	{
		return Path(*this) / path;
	}

	PathView PathView::filename() const
	{
		if (empty() || m_path == "/")
			return {};

		usize begin = size();

		while (begin > 0 && m_path[begin - 1] != '/') --begin;
		return PathView(StringView(m_path.data() + begin, size() - begin));
	}

	PathView PathView::extension() const
	{
		const PathView name = filename();

		if (name.empty())
			return {};

		// "." / ".." shouldn't be treated as extensions.
		// "." isn't valid in your normalized paths anyway,
		// but ".." can exist in relative paths.
		if (name == "." || name == "..")
			return {};

		for (usize i = name.size(); i > 0; --i)
		{
			if (name.m_path[i - 1] != '.')
				continue;

			const usize dot = i - 1;

			// ".gitignore" => no extension
			if (dot == 0)
				return {};

			return PathView(StringView(name.data() + dot, name.size() - dot));
		}

		return {};
	}

	PathView PathView::stem() const
	{
		const PathView name = filename();

		if (name.empty())
			return {};

		const PathView ext = name.extension();

		if (ext.empty())
			return name;

		return PathView(StringView(name.data(), name.size() - ext.size()));
	}

	PathView PathView::parent() const
	{
		if (empty())
			return {};

		if (m_path == "/")
			return *this;

		usize pos = size();

		while (pos > 0 && m_path[pos - 1] != '/') --pos;

		// No separator:
		//
		// "foo" -> ""
		if (pos == 0)
			return {};

		// "/foo" -> "/"
		if (pos == 1)
			return PathView(StringView(m_path.data(), 1));

		// "/foo/bar" -> "/foo"
		// "foo/bar"  -> "foo"
		return PathView(StringView(m_path.data(), pos - 1));
	}

	bool PathView::starts_with(PathView prefix) const
	{
		if (!m_path.starts_with(prefix.m_path))
			return false;

		if (size() == prefix.size())
			return true;

		if (prefix.empty())
			return true;

		if (prefix == "/")
			return is_absolute();

		return m_path[prefix.size()] == '/';
	}

	bool PathView::ends_with(PathView suffix) const
	{
		if (!m_path.ends_with(suffix.m_path))
			return false;

		if (size() == suffix.size())
			return true;

		if (suffix.empty())
			return true;

		const usize offset = size() - suffix.size();

		if (suffix[0] == '/')
			return true;

		return m_path[offset - 1] == '/';
	}

	bool PathView::is_parent_of(PathView other) const
	{
		if (!is_ancestor_of(other))
			return false;

		return component_count() + 1 == other.component_count();
	}

	bool PathView::is_child_of(PathView other) const
	{
		return other.is_parent_of(*this);
	}

	bool PathView::is_ancestor_of(PathView other) const
	{
		if (*this == other)
			return false;

		if (size() > other.size())
			return false;

		if (!other.m_path.starts_with(m_path))
			return false;

		// Exact same path.
		if (size() == other.size())
			return true;

		// "/" is a prefix of every absolute path.
		if (m_path == "/")
			return other.is_absolute();

		// Empty path is root of relative paths, if you want that semantic.
		if (empty())
			return other.is_relative();

		// Must end at component boundary.
		return other.m_path[size()] == '/';
	}

	usize PathView::component_count() const
	{
		if (empty() || m_path == "/")
			return 0;

		usize count = 1;

		for (usize i = is_absolute() ? 1 : 0; i < size(); ++i)
		{
			if (m_path[i] == '/')
				++count;
		}

		return count;
	}

	PathView PathView::root() const
	{
		return is_absolute() ? PathView("/") : PathView("");
	}

	PathView PathView::first_component() const
	{
		if (empty() || m_path == "/")
			return {};

		const usize begin = is_absolute() ? 1 : 0;

		usize end = begin;

		while (end < size() && m_path[end] != '/') ++end;

		return PathView(StringView(m_path.data() + begin, end - begin));
	}

	PathView PathView::last_component() const
	{
		if (empty() || m_path == "/")
			return {};

		usize begin = size();

		while (begin > 0 && m_path[begin - 1] != '/') --begin;

		return PathView(StringView(m_path.data() + begin, size() - begin));
	}

	PathView PathView::remove_prefix(PathView prefix) const
	{
		if (!starts_with(prefix))
			return *this;

		if (prefix.empty())
			return *this;

		if (prefix.size() == size())
			return {};

		usize offset = prefix.size();

		// "/foo/bar" - "/foo" -> "bar"
		// "foo/bar"  - "foo"  -> "bar"
		//
		// But:
		// "/foo/bar" - "/" -> "foo/bar"
		if (m_path[offset] == '/')
			++offset;

		return PathView(StringView(m_path.data() + offset, m_path.size() - offset));
	}

	PathView PathView::remove_suffix(PathView suffix) const
	{
		if (!ends_with(suffix))
			return *this;

		if (suffix.empty())
			return *this;

		if (suffix.size() == size())
			return {};

		usize new_size = size() - suffix.size();

		// "/foo/bar" - "bar"
		// new_size points AFTER "/foo/", so remove separator too.
		//
		// "/foo/bar" - "/bar"
		// suffix already contains separator, so keep "/foo".
		if (suffix[0] != '/')
			--new_size;

		// Special case:
		// "/foo" - "foo" -> "/"
		if (new_size == 0 && is_absolute())
			new_size = 1;

		return PathView(StringView(m_path.data(), new_size));
	}

	Path& Path::on_path_changed()
	{
#if PLATFORM_WINDOWS
		static auto transform_func = [](char ch) -> char {
			if (ch == '\\')
				return '/';
			return ch;
		};

		std::transform(m_path.begin(), m_path.end(), m_path.begin(), transform_func);
#endif

		simplify_path(m_path);

		return *this;
	}

	Path::Path() {}

	Path::Path(const Path& path) : m_path(path.m_path)
	{
		on_path_changed();
	}

	Path::Path(Path&& path) : m_path(std::move(path.m_path))
	{
		on_path_changed();
	}

	Path::Path(const PathView& path) : m_path(path.str())
	{
		on_path_changed();
	}

	Path::Path(const StringView& path) : m_path(path)
	{
		on_path_changed();
	}

	Path::Path(const char* str) : Path(StringView(str)) {}

	Path::Path(const String& str) : Path(StringView(str)) {}

	Path& Path::operator=(const Path& path)
	{
		if (this == &path)
			return *this;

		m_path = path.m_path;
		return on_path_changed();
	}

	Path& Path::operator=(Path&& path)
	{
		if (this == &path)
			return *this;

		m_path = std::move(path.m_path);
		return on_path_changed();
	}

	Path& Path::operator=(const PathView& path)
	{
		m_path = String(path.str());
		return on_path_changed();
	}

	Path& Path::operator=(const StringView& path)
	{
		m_path = String(path);
		return on_path_changed();
	}

	Path& Path::operator=(const String& path)
	{
		return (*this) = StringView(path);
	}

	Path& Path::operator=(const char* path)
	{
		if (path == nullptr)
		{
			new (this) Path();
			return *this;
		}
		return (*this) = StringView(path);
	}

	Path& Path::operator/=(const Path& path)
	{
		return (*this) /= path.view();
	}

	Path& Path::operator/=(PathView path)
	{
		if (path.empty())
			return *this;

		if (empty())
		{
			m_path = String(path.str());
			return on_path_changed();
		}

		if (!empty() && m_path.back() != Path::separator)
		{
			m_path.push_back(separator);
		}

		m_path += path.str();
		return on_path_changed();
	}

	bool Path::serialize(Archive& ar)
	{
		const bool status = ar.serialize(m_path);

		if (status && ar.is_reading())
			on_path_changed();

		return status;
	}
}// namespace Trinex
