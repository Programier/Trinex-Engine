#include <Core/etl/algorithm.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <UI/Elements/document.hpp>
#include <UI/widget.hpp>

namespace Trinex::UI
{
	namespace
	{
		class FunctionWidget : public Widget
		{
		private:
			Action m_action;

		public:
			FunctionWidget(StringView name, const WindowOptions& options, bool open, const Action& action)
			    : Widget(name, options, open), m_action(action)
			{}

			void on_render() override
			{
				m_action();
				Widget::on_render();
			}
		};
	}// namespace

	Widget* Widget::create(StringView name, const WindowOptions& options, bool open, const Action& action)
	{
		return trx_new FunctionWidget(name, options, open, action);
	}

	Widget::Widget(StringView name, const WindowOptions& options, bool open) : m_name(name), m_options(options), m_is_open(open)
	{}

	Widget::~Widget()
	{
		for (DocumentEntry& entry : m_documents)
		{
			if (entry.watch_id != 0)
			{
				rootfs()->unwatch(entry.watch_id);
			}

			entry.document->release();
		}
	}

	Document* Widget::load_document(const Path& path)
	{
		FileReader reader(path);

		if (!reader.is_open())
		{
			trinex_error(Log::Editor, "Failed to open UI document '%s'", path.c_str());
			return nullptr;
		}

		Document* document = create_document(reader.read_string());

		if (document == nullptr)
		{
			return nullptr;
		}

		for (DocumentEntry& entry : m_documents)
		{
			if (entry.document == document)
			{
				entry.path     = path;
				entry.watch_id = rootfs()->watch(
				        path, [this, document](const VFS::FileWatchEvent&) { reload_document(document); },
				        VFS::FileWatchEventType::Modified, false);
				break;
			}
		}

		return document;
	}

	Document* Widget::create_document(StringView source)
	{
		Document* document = trx_new Document();

		if (!document->load(source))
		{
			document->release();
			return nullptr;
		}

		add_document(document);
		document->release();
		return document;
	}

	bool Widget::reload_document(Document* document)
	{
		for (DocumentEntry& entry : m_documents)
		{
			if (entry.document != document)
			{
				continue;
			}

			if (entry.path.empty())
			{
				return false;
			}

			FileReader reader(entry.path);

			if (!reader.is_open())
			{
				trinex_error(Log::Editor, "Failed to reload UI document '%s'", entry.path.c_str());
				return false;
			}

			document->load(reader.read_string());
			return true;
		}

		return false;
	}

	Widget& Widget::add_document(Document* document)
	{
		if (document)
		{
			document->add_reference();
			m_documents.push_back({.document = document});
		}

		return *this;
	}

	Widget& Widget::remove_document(Document* document)
	{
		auto it = etl::find_if(m_documents.begin(), m_documents.end(),
		                       [document](const DocumentEntry& entry) { return entry.document == document; });

		if (it != m_documents.end())
		{
			if (it->watch_id != 0)
			{
				rootfs()->unwatch(it->watch_id);
			}

			m_documents.erase(it);
			document->release();
		}

		return *this;
	}

	void Widget::on_attach(Context* context) {}

	void Widget::on_deattach(Context* context) {}

	void Widget::on_open() {}

	void Widget::on_close() {}

	void Widget::on_render()
	{
		for (DocumentEntry& entry : m_documents)
		{
			Document* document = entry.document;

			if (document->is_open())
			{
				document->update();
			}
		}
	}
}// namespace Trinex::UI
