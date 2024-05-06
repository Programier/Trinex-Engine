#pragma once
#include <Core/name.hpp>

namespace Engine
{
    class Enum;

    ENGINE_EXPORT Enum* policies_enum();
    ENGINE_EXPORT PolicyID register_policy(const Name& name);
    ENGINE_EXPORT Name policy_name(PolicyID id);
    ENGINE_EXPORT PolicyID policy_id(const Name& name);
}// namespace Engine
