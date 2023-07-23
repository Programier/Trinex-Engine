#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/uniform_buffer.hpp>
#include <api.hpp>

namespace Engine
{
    UniformStructInstance::UniformStructInstance(DynamicStructBase* struct_instance, Index index)
        : DynamicStructInstanceProxy(struct_instance, index)
    {
        EngineInstance::instance()->api_interface()->create_uniform_buffer(_M_ID, nullptr, struct_instance->size());
    }

    byte* UniformStructInstance::data()
    {
        return EngineInstance::instance()->api_interface()->map_uniform_buffer(_M_ID).data();
    }

    const byte* UniformStructInstance::data() const
    {
        return EngineInstance::instance()->api_interface()->map_uniform_buffer(_M_ID).data();
    }

    UniformStructInstance& UniformStructInstance::bind(BindingIndex index)
    {
        EngineInstance::instance()->api_interface()->bind_uniform_buffer(_M_ID, index);
        return *this;
    }

    static void on_init()
    {
        register_class(Engine::UniformStruct);
    }

    static InitializeController initializer(on_init);
}// namespace Engine
