#include <Core/exception.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/string_functions.hpp>

namespace Engine
{
	const char Path::separator            = '/';
	const StringView Path::sv_separator   = "/";
	static constexpr const char* prev_dir = "../";

	size_t Path::Hash::operator()(const Path& p) const noexcept
	{
		static Engine::Hash<String> hasher;
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


		StringView view = m_path;

		Index position;

		{
			position = view.rfind(separator);
			if (position != StringView::npos)
			{
				m_filename  = view.substr(position + 1);
				m_base_path = view.substr(0, position);
			}
			else
			{
				m_base_path = {};
				m_filename  = view;
			}
		}

		{
			position = m_filename.rfind('.');

			if (position != StringView::npos)
			{
				m_extension = m_filename.substr(position);
				m_stem      = m_filename.substr(0, position);
			}
			else
			{
				m_extension = {};
				m_stem      = m_filename;
			}
		}

		return *this;
	}

	Path::Path()
	{}

	Path::Path(const Path& path) : m_path(path.m_path)
	{
		on_path_changed();
	}

	Path::Path(const Path&& path) : m_path(std::move(path.m_path))
	{
		on_path_changed();
	}

	Path::Path(const StringView& path) : m_path(path)
	{
		on_path_changed();
	}

	Path::Path(const char* str) : Path(StringView(str))
	{}

	Path::Path(const String& str) : Path(StringView(str))
	{}

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
		if (!empty() && m_path.back() != Path::separator)
		{
			m_path.push_back(separator);
		}

		m_path += path.m_path;
		return on_path_changed();
	}

	Vector<StringView> Path::split_sv() const
	{
		Index index   = 0;
		Index current = 0;

		StringView view = m_path;
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

	Vector<String> Path::split() const
	{
		Index index   = 0;
		Index current = 0;

		Vector<String> result;

		while ((current = m_path.find(Path::separator, index)) != String::npos)
		{
			result.push_back(m_path.substr(index, current - index));
			index = current + 1;
		}

		if (index != m_path.length() - 1)
			result.push_back(m_path.substr(index));

		return result;
	}

	Path Path::relative(const Path& base) const
	{
		if (base.empty())
			return *this;

		Vector<StringView> base_sv = base.split_sv();
		Vector<StringView> self_sv = split_sv();

		size_t min_len = glm::min(base_sv.size(), self_sv.size());
		Index index    = 0;

		while (index < min_len && base_sv[index] == self_sv[index]) ++index;

		String result;

		for (Index i = index, count = base_sv.size(); i < count; ++i)
		{
			result += prev_dir;
		}

		for (Index i = index, count = self_sv.size(); i < count; ++i)
		{
			result += self_sv[i];
			result.push_back(separator);
		}

		return result;
	}
}// namespace Engine
