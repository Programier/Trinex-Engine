#include <Core/etl/allocator.hpp>
#include <Core/reflection/struct.hpp>
#include <Core/string_functions.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	RHI* RHI::s_rhi = nullptr;
	
	RHI* RHI::create(const char* name)
	{
		auto decl = Strings::format("Trinex::TRINEX_RHI::{}", Strings::to_upper(name));
		void* rhi = static_cast<RHI*>(Refl::Struct::static_find(decl, Refl::FindFlags::IsRequired)->create_struct());

		trinex_verify_msg(rhi && rhi == s_rhi, "Failed to initialize RHI");
		return nullptr;
	}

	void RHI::destroy()
	{
		if (s_rhi)
		{
			s_rhi->info.struct_instance->destroy_struct(s_rhi);
			s_rhi = nullptr;
		}
	}

	RHI::RHI()
	{
		trinex_assert(s_rhi == nullptr);
		s_rhi = this;
	}

	RHI::~RHI()
	{
		s_rhi = nullptr;
	}
}// namespace Trinex
