#include <SDL3/SDL_clipboard.h>
#include <SDLPlatform/clipboard.hpp>

namespace Trinex::Platform
{
	struct ClipboardPayload {
		String mime;
		Vector<u8> data;
	};

	static const void* SDLCALL clipboard_data_callback(void* userdata, const char* mime_type, size_t* size)
	{
		auto* payload = static_cast<ClipboardPayload*>(userdata);

		if (!mime_type || payload->mime != mime_type)
		{
			*size = 0;
			return nullptr;
		}

		*size = payload->data.size();
		return payload->data.data();
	}

	static void SDLCALL clipboard_cleanup_callback(void* userdata)
	{
		trx_delete static_cast<ClipboardPayload*>(userdata);
	}

	SDLClipboard::SDLClipboard() {}

	SDLClipboard::~SDLClipboard()
	{
		release_mime_types();
	}

	SDLClipboard* SDLClipboard::instance()
	{
		static SDLClipboard clipboard;
		return &clipboard;
	}

	void SDLClipboard::release_mime_types()
	{
		if (m_mime_types)
		{
			SDL_free(m_mime_types);
			m_mime_types = nullptr;
		}

		m_mime_count = 0;
	}

	SDLClipboard& SDLClipboard::update()
	{
		release_mime_types();
		m_mime_types = SDL_GetClipboardMimeTypes(&m_mime_count);
		return *this;
	}

	usize SDLClipboard::mime_types() const
	{
		return m_mime_count;
	}

	const char* SDLClipboard::mime_type(usize idx) const
	{
		if (m_mime_types == nullptr)
			return nullptr;

		if (idx >= m_mime_count)
			return nullptr;

		return m_mime_types[idx];
	}

	bool SDLClipboard::clear()
	{
		release_mime_types();
		return SDL_ClearClipboardData();
	}

	bool SDLClipboard::load(const FunctionRef<void(const u8*, usize)>& func, const char* mime)
	{
		update();

		if (m_mime_count == 0 || m_mime_types == nullptr)
			return false;

		if (mime == nullptr)
		{
			mime = m_mime_types[0];
		}

		size_t size = 0;
		void* data  = SDL_GetClipboardData(mime, &size);

		if (!data)
			return false;

		func(static_cast<const u8*>(data), static_cast<usize>(size));

		SDL_free(data);
		return true;
	}

	bool SDLClipboard::store(const void* data, usize size, const char* mime)
	{
		if (!data || size == 0 || mime == nullptr)
			return false;

		auto* payload = trx_new ClipboardPayload;

		payload->mime = mime;
		payload->data.assign(static_cast<const u8*>(data), static_cast<const u8*>(data) + size);

		const char* mime_types[] = {payload->mime.c_str()};

		if (!SDL_SetClipboardData(clipboard_data_callback, clipboard_cleanup_callback, payload, mime_types, 1))
		{
			trx_delete payload;
			return false;
		}

		return true;
	}

}// namespace Trinex::Platform
