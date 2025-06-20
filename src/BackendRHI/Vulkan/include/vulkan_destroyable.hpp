#pragma once
#include <Core/memory.hpp>

namespace Engine
{
	struct RHI_Object;
	void trinex_vulkan_deferred_destroy(RHI_Object* object);

	template<typename T>
	class VulkanDeferredDestroy : public T
	{
	public:
		using T::T;

		void release() override
		{
			if (T::m_references > 0)
				--T::m_references;

			if (T::m_references == 0)
			{
				trinex_vulkan_deferred_destroy(this);
			}
		}

		void destroy() override { Engine::release(this); }
	};
}// namespace Engine
