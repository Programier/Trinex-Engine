#include <Core/archive.hpp>
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
    implement_struct(DepthTestInfo, Engine::Pipeline, ).push([]() {
        using DTI    = Pipeline::DepthTestInfo;
        Struct* self = Struct::static_find("Engine::Pipeline::DepthTestInfo", true);

        self->add_properties(
                new EnumProperty("Func", "Depth compare function", &DTI::func, Enum::find("Engine::DepthFunc", true)),
                new FloatProperty("Min depth bound", "Min depth bound", &DTI::min_depth_bound),
                new FloatProperty("Max depth bound", "Max depth bound", &DTI::max_depth_bound),
                new BoolProperty("Enable", "Enable depth test", &DTI::enable),
                new BoolProperty("Write Enable", "Enable write to depth buffer", &DTI::write_enable),
                new BoolProperty("Enable bound test", "Enable bound test", &DTI::bound_test_enable));
    });

    implement_struct(FaceInfo, Engine::Pipeline::StencilTestInfo, ).push([]() {
        using FI                = Pipeline::StencilTestInfo::FaceInfo;
        Enum* stencil_op_enum   = Enum::find("Engine::StencilOp", true);
        Enum* compare_func_enum = Enum::find("Engine::CompareFunc", true);
        Struct* self            = Struct::static_find("Engine::Pipeline::StencilTestInfo::FaceInfo", true);

        self->add_properties(new EnumProperty("Fail", "Operation on fail", &FI::fail, stencil_op_enum),
                             new EnumProperty("Depth pass", "Operation on depth pass", &FI::depth_pass, stencil_op_enum),
                             new EnumProperty("Depth Fail", "Operation on depth fail", &FI::depth_fail, stencil_op_enum),
                             new EnumProperty("Compare func", "Stencil compare function", &FI::compare, compare_func_enum),
                             new UnsignedIntProperty("Compare mask", "Stencil compare mask", &FI::compare_mask),
                             new UnsignedIntProperty("Write mask", "Stencil write mask", &FI::write_mask),
                             new IntProperty("Reference", "Stencil reference", &FI::reference));
    });

    implement_struct(StencilTestInfo, Engine::Pipeline, ).push([]() {
        using STI = Pipeline::StencilTestInfo;

        Struct* self      = Struct::static_find("Engine::Pipeline::StencilTestInfo", true);
        Struct* face_info = Struct::static_find("Engine::Pipeline::StencilTestInfo::FaceInfo", true);

        self->add_properties(new BoolProperty("Enable", "Enable stencil test", &STI::enable),
                             new StructProperty("Front", "Stencil parameters for front face", &STI::front, face_info),
                             new StructProperty("Back", "Stencil parameters for back face", &STI::back, face_info));
    });

    implement_struct(AssemblyInfo, Engine::Pipeline, ).push([]() {
        using AI = Pipeline::AssemblyInfo;

        Struct* self = Struct::static_find("Engine::Pipeline::AssemblyInfo", true);

        self->add_properties(new EnumProperty("Primitive Topology", "Primitive types which will be rendered by this pipeline",
                                              &AI::primitive_topology, Enum::find("Engine::PrimitiveTopology", true)),
                             new BoolProperty("Enable restart", "Enable primitive restart", &AI::primitive_restart_enable));
    });

    implement_struct(RasterizerInfo, Engine::Pipeline, ).push([]() {
        using RI = Pipeline::RasterizerInfo;

        Struct* self = Struct::static_find("Engine::Pipeline::RasterizerInfo", true);

        self->add_properties(
                new FloatProperty("Depth bias const factor", "Depth bias const factor", &RI::depth_bias_const_factor),
                new FloatProperty("Depth bias clamp", "Depth bias clamp", &RI::depth_bias_clamp),
                new FloatProperty("Depth bias slope factor", "Depth bias slope factor", &RI::depth_bias_slope_factor),
                new FloatProperty("Line width", "Width of line which will be rendered by this material", &RI::line_width),

                new BoolProperty("Enable depth bias", "Enable depth bias", &RI::depth_bias_enable),
                new BoolProperty("Enable discard", "If true then shaders can use discard keyword", &RI::discard_enable),
                new BoolProperty("Enable depth clamp", "Enable depth clamp", &RI::depth_clamp_enable),

                new EnumProperty("Polygon mode", "Polygon Mode", &RI::polygon_mode, Enum::find("Engine::PolygonMode", true)),
                new EnumProperty("Cull mode", "Cull Mode", &RI::cull_mode, Enum::find("Engine::CullMode", true)),
                new EnumProperty("Front face", "Front face", &RI::front_face, Enum::find("Engine::FrontFace", true)));
    });


    implement_struct(ColorBlendAttachmentState, Engine::Pipeline, ).push([]() {
        using CBAS = Pipeline::ColorBlendAttachmentState;

        Struct* self     = Struct::static_find("Engine::Pipeline::ColorBlendAttachmentState", true);
        Enum* blend_func = Enum::find("Engine::BlendFunc", true);
        Enum* blend_op   = Enum::find("Engine::BlendOp", true);

        self->add_properties(
                new BoolProperty("Enable", "Enable blending", &CBAS::enable),
                new EnumProperty("Src color func", "Src color func", &CBAS::src_color_func, blend_func),
                new EnumProperty("Dst color func", "Dst color func", &CBAS::dst_color_func, blend_func),
                new EnumProperty("Color operator", "Color operator", &CBAS::color_op, blend_op),

                new EnumProperty("Src alpha func", "Src alpha func", &CBAS::src_alpha_func, blend_func),
                new EnumProperty("Dst alpha func", "Dst alpha func", &CBAS::dst_alpha_func, blend_func),
                new EnumProperty("Alpha operator", "Alpha operator", &CBAS::alpha_op, blend_op),

                new EnumProperty("Color mask", "Color mask", &CBAS::color_mask, Enum::find("Engine::ColorComponentMask", true)));
    });

    implement_struct(ColorBlendingInfo, Engine::Pipeline, ).push([]() {
        using CBI = Pipeline::ColorBlendingInfo;

        Struct* self = Struct::static_find("Engine::Pipeline::ColorBlendingInfo", true);

        Struct* attachment_state = Struct::static_find("Engine::Pipeline::ColorBlendAttachmentState", true);

        self->add_properties(
                new ArrayProperty("Blend attachments", "Blend attachments", &CBI::blend_attachment,
                                  new StructArrayElementProperty<Pipeline::ColorBlendingInfo>(attachment_state)),

                new EnumProperty("Logic operator", "Logic operator", &CBI::logic_op, Enum::find("Engine::LogicOp", true)),
                new Vec4Property("Blend constants", "Blend constant values", &CBI::blend_constants),
                new BoolProperty("Enable logic operator", "Enable logic operator", &CBI::logic_op_enable));
    });


    Pipeline::Pipeline()
    {
        vertex_shader = Object::new_instance<VertexShader>();
        vertex_shader->flags(Object::IsAvailableForGC, false);

        fragment_shader = Object::new_instance<FragmentShader>();
        fragment_shader->flags(Object::IsAvailableForGC, false);
    }

    Pipeline::~Pipeline()
    {
        delete vertex_shader;
        delete fragment_shader;
    }

    Pipeline& Pipeline::rhi_create()
    {
        vertex_shader->rhi_create();
        fragment_shader->rhi_create();
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

        if (vertex_shader->global_ubo_location.is_valid())
        {
            ubo->rhi_bind(vertex_shader->global_ubo_location);
        }

        if (fragment_shader->global_ubo_location.is_valid())
        {
            ubo->rhi_bind(fragment_shader->global_ubo_location);
        }

        return *this;
    }

    bool Pipeline::archive_process(class Archive& archive)
    {
        if (!Super::archive_process(archive))
            return false;

        archive& depth_test;
        archive& stencil_test;
        archive& input_assembly;
        archive& rasterizer;

        archive& color_blending.blend_attachment;
        archive& color_blending.blend_constants;
        archive& color_blending.logic_op;
        archive& color_blending.logic_op_enable;

        return archive;
    }

    implement_class(Pipeline, Engine, 0);
    implement_initialize_class(Pipeline)
    {
        Class* self = static_class_instance();

        self->add_properties(new StructProperty("Depth Test", "Depth Test properties", &Pipeline::depth_test,
                                                Struct::static_find("Engine::Pipeline::DepthTestInfo", true)),
                             new StructProperty("Stencil Test", "Stencil Test properties", &Pipeline::stencil_test,
                                                Struct::static_find("Engine::Pipeline::StencilTestInfo", true)),
                             new StructProperty("Assembly Input", "Assembly Input", &Pipeline::input_assembly,
                                                Struct::static_find("Engine::Pipeline::AssemblyInfo", true)),
                             new StructProperty("Rasterizer", "Rasterizer properties", &Pipeline::rasterizer,
                                                Struct::static_find("Engine::Pipeline::RasterizerInfo", true)),
                             new StructProperty("Color blending", "Blending properties", &Pipeline::color_blending,
                                                Struct::static_find("Engine::Pipeline::ColorBlendingInfo", true)));
    }
}// namespace Engine
