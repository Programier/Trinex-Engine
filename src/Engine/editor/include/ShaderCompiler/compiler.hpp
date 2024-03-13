#pragma once
#include <Core/color_format.hpp>
#include <Core/object.hpp>
#include <Core/structures.hpp>


namespace Engine
{
    class Material;

    namespace ShaderCompiler
    {
        struct ShaderReflection {

            struct VertexAttribute {
                String name;
                ColorFormat format;
                VertexAttributeInputRate rate;
                VertexBufferSemantic semantic;
                byte semantic_index;
                byte count;
            };

            struct UniformMemberInfo {
                MaterialParameterType type;
                String name;
                size_t size;
                size_t offset;
            };

            Vector<VertexAttribute> attributes;
            Vector<UniformMemberInfo> uniform_member_infos;
            BindingIndex local_buffer_binding;
            size_t local_buffer_size;


            ShaderReflection& clear();
        };

        struct ShaderSource {
            Buffer vertex_code;
            Buffer fragment_code;
            ShaderReflection reflection;
        };

        ShaderSource create_opengl_shader(const String& source, const Vector<ShaderDefinition>& definitions = {},
                                          MessageList* errors = nullptr);
        ShaderSource create_opengl_shader_from_file(const StringView& relative, const Vector<ShaderDefinition>& definitions = {},
                                                    MessageList* errors = nullptr);
        ShaderSource create_vulkan_shader(const String& source, const Vector<ShaderDefinition>& definitions = {},
                                          MessageList* errors = nullptr);
        ShaderSource create_vulkan_shader_from_file(const StringView& relative, const Vector<ShaderDefinition>& definitions = {},
                                                    MessageList* errors = nullptr);


        class ShaderCompiler : public Object
        {
            declare_class(ShaderCompiler, Object);

        public:
            virtual bool compile(Material* material, ShaderSource& out_source, MessageList& errors) = 0;
        };

        class OpenGL_ShaderCompiler : public ShaderCompiler
        {
            declare_class(OpenGL_ShaderCompiler, ShaderCompiler);

        public:
            bool compile(Material* material, ShaderSource& out_source, MessageList& errors) override;
        };

        class Vulkan_ShaderCompiler : public ShaderCompiler
        {
            declare_class(Vulkan_ShaderCompiler, ShaderCompiler);

        public:
            bool compile(Material* material, ShaderSource& out_source, MessageList& errors) override;
        };
    }// namespace ShaderCompiler
}// namespace Engine
