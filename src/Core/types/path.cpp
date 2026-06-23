#include <Core/archive.hpp>
#include <Core/etl/hash.hpp>
#include <Core/math/math.hpp>
#include <Core/string_functions.hpp>
#include <Core/types/path.hpp>

namespace Trinex
{
	const char Path::separator            = '/';
	const StringView Path::sv_separator   = "/";
	static constexpr const char* prev_dir = "../";

	usize Path::Hash::operator()(const Path& p) const noexcept
	{
		static Trinex::Hash<String> hasher;
		return hasher(p.m_path);
	}

	static FORCE_INLINE void simplify_path(String& path)
	{
		static auto simplify_separators = [](char a, char b) { return a == b && a == Path::separator; };
		auto end                        = std::unique(path.begin(), path.end(), simplify_separators);
		path.erase(end, path.end());

		if (path.length() > 1 && path.back() == Path::separator)
			path.pop_back();
	}

	static FORCE_INLINE usize filename_offset_of(StringView path)
	{
		const usize position = path.rfind(Path::separator);
		return position == StringView::npos ? 0 : position + 1;
	}

	static FORCE_INLINE usize extension_offset_of(StringView filename)
	{
		const usize position = filename.rfind('.');
		return position == StringView::npos ? filename.length() : position;
	}

	static FORCE_INLINE StringView filename_of(StringView view)
	{
		return view.substr(filename_offset_of(view));
	}

	static FORCE_INLINE StringView extension_of(StringView view)
	{
		StringView name      = filename_of(view);
		const usize position = extension_offset_of(name);
		return position == name.length() ? StringView() : name.substr(position);
	}

	static FORCE_INLINE StringView stem_of(StringView view)
	{
		StringView name = filename_of(view);
		return name.substr(0, extension_offset_of(name));
	}

	static FORCE_INLINE StringView base_path_of(StringView view)
	{
		const usize position = view.rfind(Path::separator);
		return position == StringView::npos ? StringView() : view.substr(0, position);
	}

	static Vector<StringView> split_sv_of(StringView view)
	{
		usize index   = 0;
		usize current = 0;

		Vector<StringView> result;

		while ((current = view.find(Path::separator, index)) != StringView::npos)
		{
			result.push_back(view.substr(index, current - index));
			index = current + 1;
		}

		if (index != view.length())
			result.push_back(view.substr(index));

		return result;
	}

	static Path relative_of(StringView self, StringView base)
	{
		if (base.empty())
			return Path(self);

		Vector<StringView> base_sv = split_sv_of(base);
		Vector<StringView> self_sv = split_sv_of(self);

		usize min_len = Math::min(base_sv.size(), self_sv.size());
		usize index   = 0;

		while (index < min_len && base_sv[index] == self_sv[index]) ++index;

		String result;

		for (usize i = index, count = base_sv.size(); i < count; ++i)
		{
			result += prev_dir;
		}

		for (usize i = index, count = self_sv.size(); i < count; ++i)
		{
			result += self_sv[i];
			result.push_back(Path::separator);
		}

		return Path(result);
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

	Path PathView::relative(PathView base) const
	{
		return relative_of(m_path, base.m_path);
	}

	Path PathView::operator/(PathView path) const
	{
		return Path(*this) / path;
	}

	PathView PathView::filename() const
	{
		return filename_of(m_path);
	}

	PathView PathView::extension() const
	{
		return extension_of(m_path);
	}

	PathView PathView::stem() const
	{
		return stem_of(m_path);
	}

	PathView PathView::base_path() const
	{
		return base_path_of(m_path);
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

	PathView Path::split(PathView& remainder, i32 pos) const
	{
		return PathView(*this).split(remainder, pos);
	}

	Path Path::relative(const Path& base) const
	{
		return relative_of(m_path, base.str());
	}

	bool Path::serialize(Archive& ar)
	{
		const bool status = ar.serialize(m_path);

		if (status && ar.is_reading())
			on_path_changed();

		return status;
	}
}// namespace Trinex
