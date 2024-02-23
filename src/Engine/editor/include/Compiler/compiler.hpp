#pragma once
#include <Core/object.hpp>


namespace Engine
{
    class MaterialNode;
    class MaterialPin;
    class MaterialInputPin;
    class MaterialOutputPin;
    class Texture2D;
    class Sampler;

    enum class DecomposeVectorComponent
    {
        X = 0,
        Y = 1,
        Z = 2,
        W = 3
    };

    class ShaderCompilerBase : public Object
    {
        declare_class(ShaderCompilerBase, Object);

    public:
        virtual bool compile(class VisualMaterial* material, MessageList& errors) = 0;
    };

    class ShaderCompiler : public ShaderCompilerBase
    {
        declare_class(ShaderCompiler, ShaderCompilerBase);

    public:
        // MATH
        virtual size_t sin(MaterialInputPin*)                              = 0;
        virtual size_t cos(MaterialInputPin*)                              = 0;
        virtual size_t tan(MaterialInputPin*)                              = 0;
        virtual size_t normalize(MaterialInputPin*)                        = 0;
        virtual size_t dot(MaterialInputPin* pin1, MaterialInputPin* pin2) = 0;
        virtual size_t pow(MaterialInputPin* pin1, MaterialInputPin* pin2) = 0;
        virtual size_t floor(MaterialInputPin*)                            = 0;

        // OPERATORS
        virtual size_t bool_op(MaterialInputPin* pin)   = 0;
        virtual size_t int_op(MaterialInputPin* pin)    = 0;
        virtual size_t uint_op(MaterialInputPin* pin)   = 0;
        virtual size_t float_op(MaterialInputPin* pin)  = 0;
        virtual size_t bvec2_op(MaterialInputPin* pin)  = 0;
        virtual size_t bvec3_op(MaterialInputPin* pin)  = 0;
        virtual size_t bvec4_op(MaterialInputPin* pin)  = 0;
        virtual size_t ivec2_op(MaterialInputPin* pin)  = 0;
        virtual size_t ivec3_op(MaterialInputPin* pin)  = 0;
        virtual size_t ivec4_op(MaterialInputPin* pin)  = 0;
        virtual size_t uvec2_op(MaterialInputPin* pin)  = 0;
        virtual size_t uvec3_op(MaterialInputPin* pin)  = 0;
        virtual size_t uvec4_op(MaterialInputPin* pin)  = 0;
        virtual size_t vec2_op(MaterialInputPin* pin)   = 0;
        virtual size_t vec3_op(MaterialInputPin* pin)   = 0;
        virtual size_t vec4_op(MaterialInputPin* pin)   = 0;
        virtual size_t color3_op(MaterialInputPin* pin) = 0;
        virtual size_t color4_op(MaterialInputPin* pin) = 0;

        virtual size_t add(MaterialInputPin*, MaterialInputPin*)                                                  = 0;
        virtual size_t sub(MaterialInputPin*, MaterialInputPin*)                                                  = 0;
        virtual size_t mul(MaterialInputPin*, MaterialInputPin*)                                                  = 0;
        virtual size_t div(MaterialInputPin*, MaterialInputPin*)                                                  = 0;
        virtual size_t construct_vec2(MaterialInputPin*, MaterialInputPin*)                                       = 0;
        virtual size_t construct_vec3(MaterialInputPin*, MaterialInputPin*, MaterialInputPin*)                    = 0;
        virtual size_t construct_vec4(MaterialInputPin*, MaterialInputPin*, MaterialInputPin*, MaterialInputPin*) = 0;
        virtual size_t decompose_vec(MaterialInputPin*, DecomposeVectorComponent component)                       = 0;
        virtual size_t construct_mat3(MaterialInputPin*, MaterialInputPin*, MaterialInputPin*)                    = 0;
        virtual size_t construct_mat4(MaterialInputPin*, MaterialInputPin*, MaterialInputPin*, MaterialInputPin*) = 0;


        // GLOBALS
        //        vec4 viewport;
        //        vec4 scissor;
        //        vec3 camera_location;
        //        vec3 camera_forward;
        //        vec3 camera_right;
        //        vec3 camera_up;
        //        vec2 depth_range;


        virtual size_t projection()   = 0;
        virtual size_t view()         = 0;
        virtual size_t projview()     = 0;
        virtual size_t inv_projview() = 0;
        virtual size_t model()        = 0;

        virtual size_t camera_location() = 0;

        virtual size_t render_target_size()     = 0;
        virtual size_t time()                   = 0;
        virtual size_t gamma()                  = 0;
        virtual size_t delta_time()             = 0;
        virtual size_t fov()                    = 0;
        virtual size_t ortho_width()            = 0;
        virtual size_t ortho_height()           = 0;
        virtual size_t near_clip_plane()        = 0;
        virtual size_t far_clip_plane()         = 0;
        virtual size_t aspect_ratio()           = 0;
        virtual size_t camera_projection_mode() = 0;
        virtual size_t frag_coord()             = 0;

        // Constants

        virtual size_t bool_constant(void* value)   = 0;
        virtual size_t int_constant(void* value)    = 0;
        virtual size_t uint_constant(void* value)   = 0;
        virtual size_t float_constant(void* value)  = 0;
        virtual size_t bvec2_constant(void* value)  = 0;
        virtual size_t bvec3_constant(void* value)  = 0;
        virtual size_t bvec4_constant(void* value)  = 0;
        virtual size_t ivec2_constant(void* value)  = 0;
        virtual size_t ivec3_constant(void* value)  = 0;
        virtual size_t ivec4_constant(void* value)  = 0;
        virtual size_t uvec2_constant(void* value)  = 0;
        virtual size_t uvec3_constant(void* value)  = 0;
        virtual size_t uvec4_constant(void* value)  = 0;
        virtual size_t vec2_constant(void* value)   = 0;
        virtual size_t vec3_constant(void* value)   = 0;
        virtual size_t vec4_constant(void* value)   = 0;
        virtual size_t color3_constant(void* value) = 0;
        virtual size_t color4_constant(void* value) = 0;
        //        virtual size_t mat3_constant()              = 0;
        //        virtual size_t mat4_constant()              = 0;

        // Dynamic Parameters
        virtual size_t bool_parameter(const String& name, void* data)   = 0;
        virtual size_t int_parameter(const String& name, void* data)    = 0;
        virtual size_t uint_parameter(const String& name, void* data)   = 0;
        virtual size_t float_parameter(const String& name, void* data)  = 0;
        virtual size_t bvec2_parameter(const String& name, void* data)  = 0;
        virtual size_t bvec3_parameter(const String& name, void* data)  = 0;
        virtual size_t bvec4_parameter(const String& name, void* data)  = 0;
        virtual size_t ivec2_parameter(const String& name, void* data)  = 0;
        virtual size_t ivec3_parameter(const String& name, void* data)  = 0;
        virtual size_t ivec4_parameter(const String& name, void* data)  = 0;
        virtual size_t uvec2_parameter(const String& name, void* data)  = 0;
        virtual size_t uvec3_parameter(const String& name, void* data)  = 0;
        virtual size_t uvec4_parameter(const String& name, void* data)  = 0;
        virtual size_t vec2_parameter(const String& name, void* data)   = 0;
        virtual size_t vec3_parameter(const String& name, void* data)   = 0;
        virtual size_t vec4_parameter(const String& name, void* data)   = 0;
        virtual size_t color3_parameter(const String& name, void* data) = 0;
        virtual size_t color4_parameter(const String& name, void* data) = 0;

        // Texture
        virtual size_t texture_2d(class Engine::Texture2D* texture, MaterialInputPin* sampler, MaterialInputPin* uv) = 0;
        virtual size_t sampler(class Engine::Sampler* sampler)                                                       = 0;
        virtual size_t base_color_texture(MaterialInputPin* sampler, MaterialInputPin* uv)                           = 0;
        virtual size_t position_texture(MaterialInputPin* sampler, MaterialInputPin* uv)                             = 0;
        virtual size_t normal_texture(MaterialInputPin* sampler, MaterialInputPin* uv)                               = 0;
        virtual size_t emissive_texture(MaterialInputPin* sampler, MaterialInputPin* uv)                             = 0;
        virtual size_t data_buffer_texture(MaterialInputPin* sampler, MaterialInputPin* uv)                          = 0;
        virtual size_t scene_output_texture(MaterialInputPin* sampler, MaterialInputPin* uv)                         = 0;

        // Shader outputs
        virtual size_t vertex_output_screen_space_position(MaterialInputPin*) = 0;
        virtual size_t vertex_output_world_position(MaterialInputPin*)        = 0;
        virtual size_t vertex_output_uv0(MaterialInputPin*)                   = 0;
        virtual size_t vertex_output_uv1(MaterialInputPin*)                   = 0;
        virtual size_t vertex_output_world_normal(MaterialInputPin*)          = 0;
        virtual size_t vertex_output_world_tangent(MaterialInputPin*)         = 0;
        virtual size_t vertex_output_world_bitangent(MaterialInputPin*)       = 0;
        virtual size_t vertex_output_color(MaterialInputPin*)                 = 0;

        virtual size_t fragment_output_base_color(MaterialInputPin*) = 0;
        virtual size_t fragment_output_metalic(MaterialInputPin*)    = 0;
        virtual size_t fragment_output_specular(MaterialInputPin*)   = 0;
        virtual size_t fragment_output_roughness(MaterialInputPin*)  = 0;
        virtual size_t fragment_output_emissive(MaterialInputPin*)   = 0;
        virtual size_t fragment_output_opacity(MaterialInputPin*)    = 0;
        virtual size_t fragment_output_position(MaterialInputPin*)   = 0;
        virtual size_t fragment_output_normal(MaterialInputPin*)     = 0;


        // Inputs
        virtual size_t vertex_position_attribute(byte index) = 0;
        virtual size_t vertex_normal_attribute(byte index)   = 0;
        virtual size_t vertex_uv_attribute(byte index)       = 0;
    };
}// namespace Engine
