#pragma once

namespace Trinex
{
	class RHI;
	class RHIObject;
	class RHIGarbageGroup;

	class RHIGarbageCollector final
	{
	private:
		RHIGarbageGroup* m_root;

	private:
		RHIGarbageCollector(RHI* rhi);

	public:
		static RHIGarbageCollector* instance(RHI* rhi);
		static RHIGarbageCollector* instance();
		friend class RHI;

	public:
		RHIGarbageGroup* create_group(u32 delay = 0);

		void add(RHIObject* object, RHIGarbageGroup* group = nullptr);
		RHIObject* take(RHIGarbageGroup* group);

		void update();
	};
} // namespace Trinex
