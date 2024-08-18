#pragma once
#include <Core/export.hpp>

namespace Engine
{
	class ENGINE_EXPORT Renderer
	{
		Renderer* m_renderer = nullptr;

	public:
		Renderer& init();
		Renderer& begin_render_callback(void (*callback)());

		friend class EngineInstance;
	};
}// namespace Engine
