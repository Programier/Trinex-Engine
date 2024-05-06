#include <Core/engine.hpp>
#include <Engine/Render/command_buffer.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    static RHI* rhi()
    {
        static RHI* instance = engine_instance->rhi();
        return instance;
    }

#define declare_command_one_param(command_name, type1, name1, code)                                                              \
    class command_name##Command : public ExecutableObject                                                                        \
    {                                                                                                                            \
        type1 name1;                                                                                                             \
                                                                                                                                 \
    public:                                                                                                                      \
        command_name##Command(type1 var1) : name1(var1)                                                                          \
        {}                                                                                                                       \
                                                                                                                                 \
        int_t execute() override                                                                                                 \
        {                                                                                                                        \
            code;                                                                                                                \
            return sizeof(command_name##Command);                                                                                \
        }                                                                                                                        \
    };

#define declare_command_two_param(command_name, type1, name1, type2, name2, code)                                                \
    class command_name##Command : public ExecutableObject                                                                        \
    {                                                                                                                            \
        type1 name1;                                                                                                             \
        type2 name2;                                                                                                             \
                                                                                                                                 \
    public:                                                                                                                      \
        command_name##Command(type1 var1, type2 var2) : name1(var1), name2(var2)                                                 \
        {}                                                                                                                       \
                                                                                                                                 \
        int_t execute() override                                                                                                 \
        {                                                                                                                        \
            code;                                                                                                                \
            return sizeof(command_name##Command);                                                                                \
        }                                                                                                                        \
    };

#define declare_command_three_param(command_name, type1, name1, type2, name2, type3, name3, code)                                \
    class command_name##Command : public ExecutableObject                                                                        \
    {                                                                                                                            \
        type1 name1;                                                                                                             \
        type2 name2;                                                                                                             \
        type3 name3;                                                                                                             \
                                                                                                                                 \
    public:                                                                                                                      \
        command_name##Command(type1 var1, type2 var2, type3 var3) : name1(var1), name2(var2), name3(var3)                        \
        {}                                                                                                                       \
                                                                                                                                 \
        int_t execute() override                                                                                                 \
        {                                                                                                                        \
            code;                                                                                                                \
            return sizeof(command_name##Command);                                                                                \
        }                                                                                                                        \
    };

#define declare_command_four_param(command_name, type1, name1, type2, name2, type3, name3, type4, name4, code)                   \
    class command_name##Command : public ExecutableObject                                                                        \
    {                                                                                                                            \
        type1 name1;                                                                                                             \
        type2 name2;                                                                                                             \
        type3 name3;                                                                                                             \
        type4 name4;                                                                                                             \
                                                                                                                                 \
    public:                                                                                                                      \
        command_name##Command(type1 var1, type2 var2, type3 var3, type4 var4)                                                    \
            : name1(var1), name2(var2), name3(var3), name4(var4)                                                                 \
        {}                                                                                                                       \
                                                                                                                                 \
        int_t execute() override                                                                                                 \
        {                                                                                                                        \
            code;                                                                                                                \
            return sizeof(command_name##Command);                                                                                \
        }                                                                                                                        \
    };

    declare_command_two_param(Draw, size_t, vertices_count, size_t, vertices_offset,
                              rhi()->draw(vertices_count, vertices_offset));
    declare_command_three_param(DrawIndexed, size_t, indices_count, size_t, indices_offset, size_t, vertices_offset,
                                rhi()->draw_indexed(indices_count, indices_offset, vertices_offset));
    declare_command_three_param(DrawInstanced, size_t, vertices_count, size_t, vertices_offset, size_t, instances,
                                rhi()->draw_instanced(vertices_count, vertices_offset, instances));
    declare_command_four_param(DrawIndexedInstanced, size_t, indices_count, size_t, indices_offset, size_t, vertices_offset,
                               size_t, instances,
                               rhi()->draw_indexed_instanced(indices_count, indices_offset, vertices_offset, instances));
    declare_command_two_param(BindMaterial, MaterialInterface*, interface, SceneComponent*, component,
                              interface->apply(component));

    declare_command_three_param(BindVertexBuffer, VertexBuffer*, buffer, byte, stream, size_t, offset,
                                buffer->rhi_bind(stream, offset));

    declare_command_two_param(BindIndexBuffer, IndexBuffer*, buffer, size_t, offset, buffer->rhi_bind(offset));

    CommandBufferLayer& CommandBufferLayer::draw(size_t vertices_count, size_t vertices_offset)
    {
        create_command<DrawCommand>(vertices_count, vertices_offset);
        return *this;
    }

    CommandBufferLayer& CommandBufferLayer::draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset)
    {
        create_command<DrawIndexedCommand>(indices_count, indices_offset, vertices_offset);
        return *this;
    }

    CommandBufferLayer& CommandBufferLayer::draw_instanced(size_t vertex_count, size_t vertices_offset, size_t instances)
    {
        create_command<DrawInstancedCommand>(vertex_count, vertices_offset, instances);
        return *this;
    }

    CommandBufferLayer& CommandBufferLayer::draw_indexed_instanced(size_t indices_count, size_t indices_offset,
                                                                   size_t vertices_offset, size_t instances)
    {
        create_command<DrawIndexedInstancedCommand>(indices_count, indices_offset, vertices_offset, instances);
        return *this;
    }

    CommandBufferLayer& CommandBufferLayer::bind_material(class MaterialInterface* material, SceneComponent* component)
    {
        create_command<BindMaterialCommand>(material, component);
        return *this;
    }

    CommandBufferLayer& CommandBufferLayer::bind_vertex_buffer(class VertexBuffer* buffer, byte stream, size_t offset)
    {
        create_command<BindVertexBufferCommand>(buffer, stream, offset);
        return *this;
    }

    CommandBufferLayer& CommandBufferLayer::bind_index_buffer(class IndexBuffer* buffer, size_t offset)
    {
        create_command<BindIndexBufferCommand>(buffer, offset);
        return *this;
    }

    CommandBufferLayer& CommandBufferLayer::clear()
    {
        SceneLayer::clear();
        m_allocated = 0;
        return *this;
    }

    CommandBufferLayer& CommandBufferLayer::render(SceneRenderer* renderer, RenderTargetBase* rt)
    {
        size_t offset = 0;
        byte* data    = m_commands.data();

        while (offset < m_allocated)
        {
            ExecutableObject* object = reinterpret_cast<ExecutableObject*>(data + offset);
            offset += static_cast<size_t>(object->execute());
        }
        return *this;
    }

}// namespace Engine
