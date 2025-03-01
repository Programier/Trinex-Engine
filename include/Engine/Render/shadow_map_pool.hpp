#pragma once
#include <Core/etl/singletone.hpp>
#include <Core/pointer.hpp>
#include <Systems/system.hpp>

namespace Engine
{
	class RenderSurface;

	class ENGINE_EXPORT ShadowMapPool : public Singletone<ShadowMapPool, Engine::System>
	{
		declare_class(ShadowMapPool, System);

	private:
		struct Entry {
			Entry* next    = nullptr;
			uint64_t frame = 0;
			Pointer<RenderSurface> surface;
		};

		Entry* m_first = nullptr;
		Entry* m_last  = nullptr;

		ShadowMapPool& create() override;
		ShadowMapPool& update(float dt) override;

	public:
		RenderSurface* request_surface();
		ShadowMapPool& return_surface(RenderSurface* surface);
	};
}// namespace Engine
