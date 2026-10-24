#include <Core/etl/allocator.hpp>
#include <Core/etl/flat_map.hpp>
#include <Core/string_functions.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	namespace
	{
		struct Backend {
			RHI::CreateFunction create;
			RHI::DestroyFunction destroy;
		};

		FlatMap<RHIBackend, Backend>& backends()
		{
			static FlatMap<RHIBackend, Backend> registry;
			return registry;
		}
	}// namespace


	RHI::Registry::Registry(RHIBackend backend, CreateFunction create, DestroyFunction destroy)
	{
		auto [it, inserted] = backends().insert({backend, {create, destroy}});
		trinex_verify_msg(inserted, "RHI backend is already registered");
	}

	RHI* RHI::instance()
	{
		return nullptr;
	}

	RHI* RHI::instance(RHIBackend backend)
	{
		return nullptr;
	}
}// namespace Trinex
