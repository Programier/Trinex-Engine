#pragma once
#include <Core/types/path.hpp>
#include <UI/types.hpp>

namespace Trinex::UI
{
	class Document;

	class Widget
	{
	private:
		struct DocumentEntry {
			Document* document = nullptr;
			Path path;
			Identifier watch_id = 0;
		};

		String m_name;
		WindowOptions m_options;
		bool m_is_open = false;
		Vector<DocumentEntry> m_documents;

	public:
		static Widget* create(StringView name, const WindowOptions& options, bool open, const Action& action);
		static inline Widget* create(StringView name, const WindowOptions& options, const Action& action)
		{
			return create(name, options, false, action);
		}
		static inline Widget* create(StringView name, bool open, const Action& action) { return create(name, {}, open, action); }
		static inline Widget* create(StringView name, const Action& action) { return create(name, {}, action); }

	public:
		Widget(StringView name = "", const WindowOptions& options = {}, bool open = false);
		virtual ~Widget();

		Document* load_document(const Path& path);
		Document* create_document(StringView source);
		bool reload_document(Document* document);
		Widget& add_document(Document* document);
		Widget& remove_document(Document* document);

		virtual void on_attach(Context* context);
		virtual void on_deattach(Context* context);

		virtual void on_open();
		virtual void on_close();

		virtual void on_render();

		inline const String& name() const { return m_name; }
		inline const WindowOptions& options() const { return m_options; }
		inline bool is_open() const { return m_is_open; }

		inline Widget& name(StringView value) { trinex_this_return(m_name = value); }
		inline Widget& options(const WindowOptions& value) { trinex_this_return(m_options = value); }
		inline Widget& is_open(bool value) { trinex_this_return(m_is_open = value); }
	};

	template<typename T>
	class UniqueWidget : public T
	{
	public:
		using T::T;

		template<typename... Args>
		static UniqueWidget* create(const Args&... args)
		{
			return trx_new UniqueWidget<T>(args...);
		}

		virtual void on_close() override
		{
			T::on_close();
			trx_delete this;
		}
	};
}// namespace Trinex::UI
