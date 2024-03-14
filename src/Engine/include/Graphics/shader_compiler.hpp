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

            Vector<VertexAttribute> attributes;
            Vector<UniformMemberInfo> uniform_member_infos;

            MaterialScalarParametersInfo global_parameters_info;
            MaterialScalarParametersInfo local_parameters_info;

            ShaderReflection& clear();
        };

        struct ENGINE_EXPORT ShaderSource {
            Buffer vertex_code;
            Buffer fragment_code;
            ShaderReflection reflection;
        };

        class ENGINE_EXPORT Compiler : public Object
        {
            declare_class(Compiler, Object);

        public:
            virtual bool compile(Material* material, ShaderSource& out_source, MessageList& errors) = 0;
        };
    }// namespace ShaderCompiler
}// namespace Engine
