#pragma once
#include <Core/etl/singletone.hpp>
#include <Core/pointer.hpp>
#include <Graphics/pipeline_buffers.hpp>

namespace Engine
{
    class ENGINE_EXPORT GlobalUniformBuffer : public Singletone<GlobalUniformBuffer, EmptyClass>
    {
    public:
        struct Data {
            Matrix4f projview;
            Vector2D viewport;
            float time;
            float dt;
        };

        static Name default_pass;


    protected:
        struct Buffer {
            Data _M_data;
            Pointer<UniformBuffer> _M_buffer;
        };
        static GlobalUniformBuffer* _M_instance;
        Map<Index, Buffer> _M_buffers;
        Buffer* _M_current_buffer;
        Name _M_current_pass;

        GlobalUniformBuffer();

        Buffer* find_buffer(Name pass_name);

        static void private_rhi_update(Buffer* buffer, size_t size = sizeof(Data), size_t offset = 0);
        static void private_rhi_bind(Buffer* buffer, BindLocation location);


        GlobalUniformBuffer& update(float dt);

    public:
        GlobalUniformBuffer& allocate_new_buffer(Name pass_name);
        bool change_pass(Name pass_name);

        Name current_pass_name() const;
        Data* current_data();
        Data* data_of(Name pass_name);

        GlobalUniformBuffer& rhi_update(Name pass_name, size_t size = sizeof(Data), size_t offset = 0);
        GlobalUniformBuffer& rhi_update(size_t size = sizeof(Data), size_t offset = 0);

        GlobalUniformBuffer& rhi_bind(Name pass_name, BindLocation location);
        GlobalUniformBuffer& rhi_bind(BindLocation location);

        friend class Object;
        friend class Singletone<GlobalUniformBuffer, EmptyClass>;
        friend class RenderSystem;
    };
}// namespace Engine
