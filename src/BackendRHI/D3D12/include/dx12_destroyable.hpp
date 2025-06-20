#pragma once

namespace Engine
{
	struct RHI_Object;
	void d3d12_deferred_destroy(RHI_Object* object);

	template<typename Base>
	class D3D12_DeferredDestroyable : public Base
	{
	public:
		using Base::Base;
		void destroy() override { d3d12_deferred_destroy(this); }
	};
}// namespace Engine
