#include <Core/memory.hpp>
#include <Engine/settings.hpp>
#include <RHI/rhi_initializers.hpp>

namespace Engine
{
	RHISamplerInitializer::RHISamplerInitializer()
	    : filter(RHISamplerFilter::Point),            //
	      address_u(RHISamplerAddressMode::Repeat),   //
	      address_v(RHISamplerAddressMode::Repeat),   //
	      address_w(RHISamplerAddressMode::Repeat),   //
	      compare_func(RHICompareFunc::Always),       //
	      border_color(0, 0, 0, 255),                 //
	      anisotropy(Settings::Rendering::anisotropy),//
	      mip_lod_bias(0.0),                          //
	      min_lod(0.f),                               //
	      max_lod(std::numeric_limits<float>::max())  //
	{}

	HashIndex RHISamplerInitializer::hash() const
	{
		return memory_hash(this, sizeof(RHISamplerInitializer));
	}

	bool RHISamplerInitializer::operator==(const RHISamplerInitializer& initializer) const
	{
		return filter == initializer.filter &&            //
		       address_u == initializer.address_u &&      //
		       address_v == initializer.address_v &&      //
		       address_w == initializer.address_w &&      //
		       compare_func == initializer.compare_func &&//
		       border_color == initializer.border_color &&//
		       anisotropy == initializer.anisotropy &&    //
		       mip_lod_bias == initializer.mip_lod_bias &&//
		       min_lod == initializer.min_lod &&          //
		       max_lod == initializer.max_lod;
	}
}// namespace Engine
