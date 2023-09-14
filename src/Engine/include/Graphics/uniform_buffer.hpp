#pragma once

#include <Core/api_object.hpp>
#include <Core/constants.hpp>
#include <Core/dynamic_struct.hpp>
#include <Core/engine_types.hpp>
#include <Core/mapped_memory.hpp>

namespace Engine
{
    class ENGINE_EXPORT UniformStructInstance : public DynamicStructInstanceProxy, public ApiObjectNoBase
    {
    public:
        UniformStructInstance(DynamicStructBase* struct_instance, Index index);

        byte* data() override;
        const byte* data() const override;

        UniformStructInstance& bind(BindingIndex index);
    };

    class ENGINE_EXPORT UniformStruct : public DynamicStruct<UniformStructInstance>
    {
        declare_class(UniformStruct, DynamicStructBase);
    };

}// namespace Engine
