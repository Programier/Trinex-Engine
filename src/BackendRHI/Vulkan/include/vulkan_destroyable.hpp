#pragma once

namespace Engine
{
	class RHIObject;
	void trinex_vulkan_deferred_destroy(RHIObject* object);

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

		void destroy() override { trx_delete this; }
	};
}// namespace Engine
