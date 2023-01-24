#pragma once
#include <Core/constants.hpp>
#include <Core/export.hpp>
#include <Core/implement.hpp>
#include <Core/object.hpp>
#include <Graphics/shader.hpp>

namespace Engine
{
    class Texture;
    namespace MaterialOutputLocations
    {
        ENGINE_EXPORT extern const ArrayIndex albedo;
    }


    namespace MaterialNodes
    {
        enum class NodeType : std::size_t
        {
            MaterialOutput = 0,
            ConstantFloat,
            ConstantFloat2,
            ConstantFloat3,
            ConstantFloat4,
            ConstantInt,
            ConstantInt2,
            ConstantInt3,
            ConstantInt4,
            Min,
            Max,
            Sum,
            Diff,
            Multiply,
            Division,
            FragCoord,
            UV,
            VectorProperty,
            Texture,
            Pow,
            Sin,
            Cos,
            Tan,
            Normalize,

            COUNT
        };

        ENGINE_EXPORT struct ShaderData {
            using Variables = std::vector<std::pair<std::string, std::string>>;
            using Uniforms = std::vector<std::pair<std::string, std::string>>;
            using InParams = std::vector<std::pair<std::string, std::string>>;

            Variables variables;
            Uniforms uniforms;
            InParams in_params;
        };


        ENGINE_EXPORT class MaterialNode : public Engine::Object
        {
        protected:
            Counter _M_references = 0;
            ArrayIndex _M_variable_index = Constants::index_none;
            ArrayIndex _M_uniform_index = Constants::index_none;

            void _M_link_to(MaterialNode* node);
            void _M_destroy_link_to(MaterialNode* node);

        public:
            const NodeType node_type;
            MaterialNode(const NodeType& id);
            virtual std::string generate_code(ShaderData& compile_data) = 0;
            inline Counter references() const
            {
                return _M_references;
            }

            inline ArrayIndex variable_index()
            {
                return _M_variable_index;
            }

            inline ArrayIndex uniform_index()
            {
                return _M_uniform_index;
            }

            virtual std::string get_type() = 0;
        };


#define NODE(name) ENGINE_EXPORT class name : public MaterialNode
#define INPUT_SLOT(name)                                                                                                                                  \
private:                                                                                                                                                  \
    MaterialNode* _M_##name = nullptr;                                                                                                                    \
                                                                                                                                                          \
public:                                                                                                                                                   \
    MaterialNode* name() const;                                                                                                                           \
    void name(MaterialNode* value);


        NODE(MaterialOutput)
        {
            INPUT_SLOT(albedo);
            INPUT_SLOT(position);

        public:
            MaterialOutput();
            std::string generate_code(ShaderData & compile_data) override;
            std::string get_type() override;
        };

        NODE(ConstantFloat)
        {
        public:
            float value = 0;

            ConstantFloat(float value = 0);
            std::string generate_code(ShaderData & compile_data) override;
            std::string get_type() override;
        };

        NODE(ConstantFloat2)
        {
        public:
            float x = 0;
            float y = 0;

            ConstantFloat2(float x = 0, float y = 0);
            ConstantFloat2(const Vector2D& value);
            std::string generate_code(ShaderData & compile_data) override;
            std::string get_type() override;
        };

        NODE(ConstantFloat3)
        {
        public:
            float x = 0;
            float y = 0;
            float z = 0;

            ConstantFloat3(float x = 0, float y = 0, float z = 0);
            ConstantFloat3(const Vector3D& value);

            std::string generate_code(ShaderData & compile_data) override;
            std::string get_type() override;
        };

        NODE(ConstantFloat4)
        {
        public:
            float x = 0;
            float y = 0;
            float z = 0;
            float a = 0;

            ConstantFloat4(float x = 0, float y = 0, float z = 0, float a = 0);
            ConstantFloat4(const Vector4D& value);

            std::string generate_code(ShaderData & compile_data) override;

            std::string get_type() override;
        };

        NODE(Min)
        {
            INPUT_SLOT(value1);
            INPUT_SLOT(value2);

        public:
            Min(MaterialNode* value1 = nullptr, MaterialNode* value2 = nullptr);
            std::string generate_code(ShaderData & compile_data) override;

            std::string get_type() override;
        };

        NODE(Max)
        {
            INPUT_SLOT(value1);
            INPUT_SLOT(value2);

        public:
            Max(MaterialNode* first = nullptr, MaterialNode* second = nullptr);
            std::string generate_code(ShaderData & compile_data) override;
            std::string get_type() override;
        };

        NODE(VectorProperty)
        {
            INPUT_SLOT(vector);
            property_hpp(VectorProperty, std::string, property, "x");

        public:
            VectorProperty(MaterialNode* vector = nullptr, const std::string& property = "x");
            std::string generate_code(ShaderData & compile_data) override;

            std::string get_type() override;
        };

        NODE(Texture)
        {
            Engine::Texture* _M_texture = nullptr;
            INPUT_SLOT(UV);

        public:
            Texture(Engine::Texture* texture = nullptr);
            const Engine::Texture* texture() const;
            Texture& texture(Engine::Texture * texture);

            std::string generate_code(ShaderData & compile_data) override;
            std::string get_type() override;
        };

        NODE(UV)
        {
        public:
            UV();
            std::string generate_code(ShaderData & compile_data) override;

            std::string get_type() override;
        };

        NODE(FragCoord)
        {
        public:
            FragCoord();
            std::string generate_code(ShaderData & compile_data) override;

            std::string get_type() override;
        };

        NODE(Sum)
        {
            INPUT_SLOT(value1);
            INPUT_SLOT(value2);


        public:
            Sum(MaterialNode* first = nullptr, MaterialNode* second = nullptr);
            std::string generate_code(ShaderData & compile_data) override;
            std::string get_type() override;
        };

        NODE(Diff)
        {
            INPUT_SLOT(value1);
            INPUT_SLOT(value2);

        public:
            Diff(MaterialNode* first = nullptr, MaterialNode* second = nullptr);
            std::string generate_code(ShaderData & compile_data) override;
            std::string get_type() override;
        };

        NODE(Multiply)
        {
            INPUT_SLOT(value1);
            INPUT_SLOT(value2);

        public:
            Multiply(MaterialNode* first = nullptr, MaterialNode* second = nullptr);
            std::string generate_code(ShaderData & compile_data) override;
            std::string get_type() override;
        };


        NODE(Division)
        {
            INPUT_SLOT(value1);
            INPUT_SLOT(value2);

        public:
            Division(MaterialNode* first = nullptr, MaterialNode* second = nullptr);
            std::string generate_code(ShaderData & compile_data) override;
            std::string get_type() override;
        };

        NODE(Pow)
        {
            INPUT_SLOT(value1);
            INPUT_SLOT(value2);

        public:
            Pow(MaterialNode* first = nullptr, MaterialNode* second = nullptr);
            std::string generate_code(ShaderData & compile_data) override;
            std::string get_type() override;
        };


        NODE(ConstantInt)
        {
        public:
            int value = 0;

            ConstantInt(int value = 0);
            std::string generate_code(ShaderData & compile_data) override;
            std::string get_type() override;
        };

        NODE(ConstantInt2)
        {
        public:
            float x = 0;
            float y = 0;

            ConstantInt2(int x = 0, int y = 0);
            ConstantInt2(const IntVector2D& value);
            std::string generate_code(ShaderData & compile_data) override;
            std::string get_type() override;
        };

        NODE(ConstantInt3)
        {
        public:
            float x = 0;
            float y = 0;
            float z = 0;

            ConstantInt3(int x = 0, int y = 0, int z = 0);
            ConstantInt3(const IntVector3D& value);

            std::string generate_code(ShaderData & compile_data) override;
            std::string get_type() override;
        };

        NODE(ConstantInt4)
        {
        public:
            float x = 0;
            float y = 0;
            float z = 0;
            float a = 0;

            ConstantInt4(int x = 0, int y = 0, int z = 0, int a = 0);
            ConstantInt4(const IntVector4D& value);

            std::string generate_code(ShaderData & compile_data) override;

            std::string get_type() override;
        };

        NODE(Sin)
        {
            INPUT_SLOT(value);

        public:
            Sin(MaterialNode* value = nullptr);

            std::string generate_code(ShaderData & compile_data) override;
            std::string get_type() override;
        };


        NODE(Cos)
        {
            INPUT_SLOT(value);

        public:
            Cos(MaterialNode* value = nullptr);

            std::string generate_code(ShaderData & compile_data) override;
            std::string get_type() override;
        };

        NODE(Tan)
        {
            INPUT_SLOT(value);

        public:
            Tan(MaterialNode* value = nullptr);

            std::string generate_code(ShaderData & compile_data) override;
            std::string get_type() override;
        };


        NODE(Normalize)
        {
            INPUT_SLOT(value);

        public:
            Normalize(MaterialNode* value = nullptr);

            std::string generate_code(ShaderData & compile_data) override;
            std::string get_type() override;
        };


#undef NODE
#undef INPUT_SLOT
    }// namespace MaterialNodes


    ENGINE_EXPORT class Material : public Object
    {
    private:
        Shader _M_shader;
        PriorityIndex _M_priority = 0;

    public:
        PriorityIndex priority() const;
        Material& priority(PriorityIndex index);
        virtual Material& apply();
    };
}// namespace Engine
