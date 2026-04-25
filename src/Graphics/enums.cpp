#include <Core/reflection/enum.hpp>
#include <Graphics/enums.hpp>

namespace Trinex
{
	trinex_implement_engine_enum(MaterialDomain, Refl::Enum::IsScriptable, Surface, PostProcess);
	trinex_implement_engine_enum(MaterialDepthMode, Refl::Enum::IsScriptable, Disabled, Test, Write, TestAndWrite);
	trinex_implement_engine_enum(MaterialBlendMode, Refl::Enum::IsScriptable, Opaque, Masked, Translucent, Additive, Modulate);
}// namespace Trinex
