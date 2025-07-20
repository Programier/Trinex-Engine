#pragma once

namespace Engine
{
	struct RHIObject;
	void d3d12_deferred_destroy(RHIObject* object);

	template<typename Base>
	class D3D12_DeferredDestroyable : public Base
	{
	public:
		using Base::Base;
		void destroy() override { d3d12_deferred_destroy(this); }
	};
}// namespace Engine
