#include <Core/base_engine.hpp>
#include <Core/package.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <Engine/Render/shadow_map_pool.hpp>
#include <Engine/settings.hpp>
#include <Graphics/render_surface.hpp>
#include <Systems/engine_system.hpp>

namespace Engine
{
	static constexpr uint64_t s_shadow_map_live_threshold = 60 * 3;

#if TRINEX_DEBUG_BUILD
	static Package* s_pool_package = nullptr;
#endif

	trinex_implement_engine_class_default_init(ShadowMapPool, 0);

	ShadowMapPool& ShadowMapPool::create()
	{
		Super::create();
		system_of<EngineSystem>()->register_subsystem(this);

#if TRINEX_DEBUG_BUILD
		s_pool_package = Package::static_find_package("TrinexEngine::ShadowMapPool", true);
#endif
		return *this;
	}

	ShadowMapPool& ShadowMapPool::update(float dt)
	{
		Super::update(dt);

		uint64_t current_frame = engine_instance->frame_index();

		while (m_first && current_frame - m_first->frame > s_shadow_map_live_threshold)
		{
			Entry* next = m_first->next;
			delete m_first;
			m_first = next;
		}

		if (m_first == nullptr)
			m_last = nullptr;

		return *this;
	}

	RenderSurface* ShadowMapPool::request_surface()
	{
		if (m_first)
		{
			Entry* current = m_first;
			m_first        = m_first->next;

			if (m_first == nullptr)
				m_last = nullptr;

			RenderSurface* surface = current->surface;
			delete current;
			return surface;
		}

		RenderSurface* surface = Object::new_instance<RenderSurface>();
		surface->init(ColorFormat::ShadowDepth, {Settings::Rendering::shadow_map_size, Settings::Rendering::shadow_map_size});

#if TRINEX_DEBUG_BUILD
		surface->rename(Strings::format("ShadowSurface.{}", static_cast<void*>(surface)));
		s_pool_package->add_object(surface);
#endif
		return surface;
	}

	ShadowMapPool& ShadowMapPool::return_surface(RenderSurface* surface)
	{
		Entry* entry   = new Entry();
		entry->frame   = engine_instance->frame_index();
		entry->surface = surface;

		if (m_first == nullptr)
		{
			m_first = m_last = entry;
		}
		else
		{
			m_last->next = entry;
			m_last       = entry;
		}
		return *this;
	}

	static InitializeController initializer([]() { System::system_of<ShadowMapPool>(); });
}// namespace Engine
