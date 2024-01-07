#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/enum.hpp>
#include <Core/property.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/render_target_base.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/shader.hpp>

namespace Engine
{

    implement_struct(DepthTestInfo, Engine, ).push([]() {
        Struct* self = Struct::static_find("Engine::DepthTestInfo", true);
        self->add_properties(
                new EnumProperty("Func", "Depth compare function", &Pipeline::DepthTestInfo::func, Enum::find("Engine::DepthFunc", true)));
    });


    Pipeline& Pipeline::rhi_create()
    {
        _M_rhi_object.reset(engine_instance->rhi()->create_pipeline(this));
        return *this;
    }

    const Pipeline& Pipeline::rhi_bind() const
    {
        if (_M_rhi_object)
        {
            rhi_object<RHI_Pipeline>()->bind();
        }

        UniformBuffer* ubo = RenderTargetBase::current_target()->uniform_buffer();

        if (vertex_shader.ptr()->global_ubo_location.is_valid())
        {
            ubo->rhi_bind(vertex_shader.ptr()->global_ubo_location);
        }

        if (fragment_shader.ptr()->global_ubo_location.is_valid())
        {
            ubo->rhi_bind(fragment_shader.ptr()->global_ubo_location);
        }

        return *this;
    }

    implement_class(Pipeline, Engine, 0);
    implement_initialize_class(Pipeline)
    {
        Class* self = static_class_instance();

        Struct* it = Struct::static_find("Engine::DepthTestInfo", true);
        self->add_properties(new StructProperty("Depth Test", "Depth Test properties", &Pipeline::depth_test, it));
        //        struct DepthTestInfo {
        //            DepthFunc func             = DepthFunc::Less;
        //            float min_depth_bound      = 0.0;
        //            float max_depth_bound      = 0.0;
        //            byte enable : 1            = 1;
        //            byte write_enable : 1      = 1;
        //            byte bound_test_enable : 1 = 0;
        //        } depth_test;
    }
}// namespace Engine
