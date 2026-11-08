#pragma once
#include <Core/memory/pool.hpp>
#include <Core/tickable.hpp>

namespace Trinex
{
	class RHI;
	class RHIObject;
	class RHIGarbageGroup;

	class RHIGarbageCollector final : public Tickable
	{
	private:
		RHI* m_rhi;
		RHIGarbageGroup* m_root;
		DynamicPoolAllocator<24, 512> m_allocator;

	private:
		RHIGarbageCollector(RHI* rhi);
		~RHIGarbageCollector();

		template<typename T, typename... Args>
		inline T* allocate(Args&&... args)
		{
			T* place = m_allocator.allocate<T>();
			return ::new (place) T(static_cast<Args&&>(args)...);
		}

		inline void free(void* ptr) { return m_allocator.free(ptr); }

	public:
		static RHIGarbageCollector* instance(RHI* rhi);
		static RHIGarbageCollector* instance();
		friend class RHI;

	public:
		RHIGarbageGroup* create_group(u64 delay);
		void destroy_group(RHIGarbageGroup* group);

		void add(RHIObject* object, RHIGarbageGroup* group = nullptr);
		RHIObject* take(RHIGarbageGroup* group);
		void end_frame(u64 frame = 0) final override;

		inline RHI* rhi() const { return m_rhi; }
		friend class RHIGarbageGroup;
	};
} // namespace Trinex
