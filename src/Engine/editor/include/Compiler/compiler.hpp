#pragma once
#include <Core/object.hpp>


namespace Engine
{
    class MaterialNode;
    class MaterialPin;
    class MaterialInputPin;
    class MaterialOutputPin;
    class Texture2D;

    enum class DecomposeVectorComponent
    {
        X = 0,
        Y = 1,
        Z = 2,
        W = 3
    };

    class ShaderCompiler : public Object
    {
        declare_class(ShaderCompiler, Object);

    public:
        virtual bool compile(class VisualMaterial* material, MessageList& errors) = 0;


        // MATH
        virtual size_t sin(MaterialInputPin*) = 0;
        virtual size_t cos(MaterialInputPin*) = 0;
        virtual size_t tan(MaterialInputPin*) = 0;

        // OPERATORS
        virtual size_t add(MaterialInputPin*, MaterialInputPin*)                                                  = 0;
        virtual size_t sub(MaterialInputPin*, MaterialInputPin*)                                                  = 0;
        virtual size_t mul(MaterialInputPin*, MaterialInputPin*)                                                  = 0;
        virtual size_t div(MaterialInputPin*, MaterialInputPin*)                                                  = 0;
        virtual size_t construct_vec2(MaterialInputPin*, MaterialInputPin*)                                       = 0;
        virtual size_t construct_vec3(MaterialInputPin*, MaterialInputPin*, MaterialInputPin*)                    = 0;
        virtual size_t construct_vec4(MaterialInputPin*, MaterialInputPin*, MaterialInputPin*, MaterialInputPin*) = 0;
        virtual size_t decompose_vec(MaterialInputPin*, DecomposeVectorComponent component)                       = 0;


        // GLOBALS
        //        vec4 viewport;
        //        vec4 scissor;
        //        vec3 camera_location;
        //        vec3 camera_forward;
        //        vec3 camera_right;
        //        vec3 camera_up;
        //        vec2 size;
        //        vec2 depth_range;
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

        // Texture
        virtual size_t texture_2d(class Engine::Texture2D* texture, MaterialInputPin* sampler, MaterialInputPin* uv) = 0;

        virtual size_t base_color(MaterialInputPin*) = 0;
    };
}// namespace Engine
