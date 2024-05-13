#pragma once
#include <Core/object.hpp>
#include <Core/structures.hpp>

namespace Engine
{
    class Material;

    namespace ShaderCompiler
    {
        struct ENGINE_EXPORT ShaderReflection {

            struct ENGINE_EXPORT VertexAttribute {
                String name;
                VertexBufferElementType type;
                VertexAttributeInputRate rate;
                VertexBufferSemantic semantic;
                byte semantic_index;
            };

            Vector<VertexAttribute> attributes;
            Vector<MaterialParameterInfo> uniform_member_infos;

            MaterialScalarParametersInfo global_parameters_info;
            MaterialScalarParametersInfo local_parameters_info;

            FORCE_INLINE ShaderReflection& clear()
            {
                global_parameters_info.remove_parameters();
                local_parameters_info.remove_parameters();
                uniform_member_infos.clear();
                attributes.clear();
                return *this;
            }
        };

        struct ENGINE_EXPORT ShaderSource {
            Buffer vertex_code;
            Buffer tessellation_control_code;
            Buffer tessellation_code;
            Buffer geometry_code;
            Buffer fragment_code;
            Buffer compute_code;
            ShaderReflection reflection;

            FORCE_INLINE bool has_valid_graphical_pipeline() const
            {
                return has_vertex_shader() && has_fragment_shader();
            }

            FORCE_INLINE bool has_valid_compute_pipeline() const
            {
                return has_compute_shader();
            }

            FORCE_INLINE bool has_vertex_shader() const
            {
                return !vertex_code.empty();
            }

            FORCE_INLINE bool has_tessellation_control_shader() const
            {
                return !tessellation_control_code.empty();
            }

            FORCE_INLINE bool has_tessellation_shader() const
            {
                return !tessellation_code.empty();
            }

            FORCE_INLINE bool has_geometry_shader() const
            {
                return !geometry_code.empty();
            }

            FORCE_INLINE bool has_fragment_shader() const
            {
                return !fragment_code.empty();
            }

            FORCE_INLINE bool has_compute_shader() const
            {
                return !compute_code.empty();
            }
        };

        class ENGINE_EXPORT Compiler : public Object
        {
            declare_class(Compiler, Object);

        public:
            virtual bool compile(Material* material, const String& slang_source, ShaderSource& out_source,
                                 MessageList& errors) = 0;
        };
    }// namespace ShaderCompiler
}// namespace Engine
