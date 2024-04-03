#pragma once
#include <Core/color_format.hpp>
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
                ColorFormat format;
                VertexAttributeInputRate rate;
                VertexBufferSemantic semantic;
                byte semantic_index;
                byte count;
            };

            struct ENGINE_EXPORT UniformMemberInfo {
                MaterialParameterType type;
                String name;
                size_t size;
                size_t offset;
            };

            struct ENGINE_EXPORT BindingObject
            {
                String name;
                BindLocation location;
                MaterialParameterType type;
            };

            Vector<VertexAttribute> attributes;
            Vector<UniformMemberInfo> uniform_member_infos;

            MaterialScalarParametersInfo global_parameters_info;
            MaterialScalarParametersInfo local_parameters_info;

            Vector<BindingObject> binding_objects;

            ShaderReflection& clear();
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
            virtual bool compile(Material* material, ShaderSource& out_source, MessageList& errors) = 0;
        };
    }// namespace ShaderCompiler
}// namespace Engine
