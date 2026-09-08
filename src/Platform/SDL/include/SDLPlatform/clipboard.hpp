#pragma once
#include <Platform/clipboard.hpp>

namespace Trinex::Platform
{
	class ENGINE_EXPORT SDLClipboard : public Clipboard
	{
	private:
		char** m_mime_types = nullptr;
		usize m_mime_count  = 0;

	private:
		SDLClipboard();
		~SDLClipboard();

		void release_mime_types();

	public:
		static SDLClipboard* instance();

		SDLClipboard& update();
		usize mime_types() const override;
		const char* mime_type(usize idx = 0) const override;

		bool clear() override;
		bool load(const FunctionRef<void(const u8*, usize)>& func, const char* mime = nullptr) override;
		bool store(const void* data, usize size, const char* mime) override;
	};
}// namespace Trinex::Platform
