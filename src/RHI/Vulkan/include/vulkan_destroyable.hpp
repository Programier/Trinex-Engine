#pragma once

namespace Engine
{
	struct RHI_Object;
	void trinex_vulkan_deferred_destroy(RHI_Object* object);

	template<typename T>
	class VulkanDeferredDestroy : public T
	{
	public:
		using T::T;

		void destroy() override { trinex_vulkan_deferred_destroy(this); }
	};
}// namespace Engine
