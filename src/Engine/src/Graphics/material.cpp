#include <Core/check.hpp>
#include <Core/destroy_controller.hpp>
#include <Core/string_format.hpp>
#include <Graphics/material.hpp>
#include <Graphics/scene.hpp>
#include <iostream>


#define NODE_TYPE(class_name) NodeType::class_name

#define gen_code(value) (value ? value->generate_code(shader_data) : "")
#define generate_output(output_param, _default) (_M_##output_param ? gen_code(_M_##output_param) : _default)
#define node_compiler(name, compile_code)                                                                              \
    std::string name::generate_code(ShaderData& shader_data)                                                           \
    {                                                                                                                  \
        auto compiler = [&]() compile_code;                                                                            \
        return compile_node(shader_data, _M_variable_index, _M_references, compiler, get_type());                      \
    }


#define INPUT_SLOT(class_name, slot_name)                                                                              \
    MaterialNode* class_name::slot_name() const                                                                        \
    {                                                                                                                  \
        return this->_M_##slot_name;                                                                                   \
    }                                                                                                                  \
    void class_name::slot_name(MaterialNode* value)                                                                    \
    {                                                                                                                  \
        _M_destroy_link_to(this->_M_##slot_name);                                                                      \
        this->_M_##slot_name = value;                                                                                  \
        _M_link_to(value);                                                                                             \
    }


namespace Engine
{
    namespace MaterialOutputLocations
    {
        ENGINE_EXPORT const ArrayIndex albedo = 0;
    }

    namespace MaterialNodes
    {
        static int get_type_priority(const std::string& type)
        {
            static std::string priorities[] = {"int", "uint", "float", "vec2", "vec3", "vec4"};
            static int count = sizeof(priorities) / sizeof(std::string);
            for (int i = 0; i < count; i++)
                if (priorities[i] == type)
                    return i;
            return -1;
        }

        template<typename Type>
        static std::string compile_node(ShaderData& data, ArrayIndex& index, Counter& references, Type compiler,
                                        std::string type = "")
        {
            std::string code_of_node;

            if (index == Constants::index_none)
            {
                code_of_node = compiler();

                if (references > 1)
                {
                    index = data.variables.size();
                    std::string variable_name = Strings::format("_var_{}", index);
                    data.variables.push_back(
                            {variable_name, Strings::format("{} {} = {};", type, variable_name, code_of_node)});
                    code_of_node = variable_name;
                }
            }
            else
            {
                return data.variables[index].first;
            }

            return code_of_node;
        }

        void MaterialNode::_M_link_to(MaterialNode* node)
        {
            if (node)
                ++node->_M_references;
        }

        void MaterialNode::_M_destroy_link_to(MaterialNode* node)
        {
            if (node)
                --node->_M_references;
        }


        MaterialNode::MaterialNode(const NodeType& type) : node_type(type)
        {}


        //////////////////////////////// NODES IMPLEMENTATION ////////////////////////////////

        //////////////////////////////// MaterialOutput IMPLEMENTATION ////////////////////////////////

        MaterialOutput::MaterialOutput() : MaterialNode(NODE_TYPE(MaterialOutput))
        {}

        INPUT_SLOT(MaterialOutput, albedo);
        INPUT_SLOT(MaterialOutput, position);

        node_compiler(MaterialOutput, {
            std::string albedo_output = generate_output(albedo, "vec4(1, 1, 1, 1)");
            std::string position_output = generate_output(position, "vec4(1, 1, 1, 1)");
            std::string result = "#version 320 es\n";

            result += "uniform (location = 0) vec4 o_albedo;\n";
            result += "uniform (location = 1) vec4 o_position;\n\n";

            for (auto& ell : shader_data.uniforms) result += Strings::format("{}\n", ell.second);
            result += "\n";

            for (auto& ell : shader_data.in_params) result += Strings::format("in {}\n", ell.second);
            result += "\n";


            result += "void main()\n{\n";

            for (auto& ell : shader_data.variables) result += Strings::format("\t{}\n", ell.second);

            result += Strings::format("\n\to_albedo = vec4({});\n", albedo_output);
            result += Strings::format("\to_position = vec3({});\n", position_output);

            return result + "}\n";
        });

        std::string MaterialOutput::get_type()
        {
            return "";
        }

        //////////////////////////////// ConstantFloat IMPLEMENTATION ////////////////////////////////

        ConstantFloat::ConstantFloat(float x) : MaterialNode(NODE_TYPE(ConstantFloat)), value(x)
        {}


        node_compiler(ConstantFloat, { return Strings::format("{}", value); });

        std::string ConstantFloat::get_type()
        {
            return "float";
        }


        //////////////////////////////// ConstantFloat2 IMPLEMENTATION ////////////////////////////////

        ConstantFloat2::ConstantFloat2(float x, float y) : MaterialNode(NODE_TYPE(ConstantFloat2)), x(x), y(y)
        {}


        ConstantFloat2::ConstantFloat2(const Vector2D& value) : ConstantFloat2(value.x, value.y)
        {}


        node_compiler(ConstantFloat2, { return Strings::format("vec2({}, {})", x, y); });

        std::string ConstantFloat2::get_type()
        {
            return "vec2";
        }

        //////////////////////////////// ConstantFloat3 IMPLEMENTATION ////////////////////////////////


        ConstantFloat3::ConstantFloat3(float x, float y, float z)
            : MaterialNode(NODE_TYPE(ConstantFloat3)), x(x), y(y), z(z)
        {}


        ConstantFloat3::ConstantFloat3(const Vector3D& value) : ConstantFloat3(value.x, value.y, value.z)
        {}


        node_compiler(ConstantFloat3, { return Strings::format("vec3({}, {}, {})", x, y, z); });

        std::string ConstantFloat3::get_type()
        {
            return "vec3";
        }

        //////////////////////////////// ConstantFloat4 IMPLEMENTATION ////////////////////////////////

        ConstantFloat4::ConstantFloat4(float x, float y, float z, float a)
            : MaterialNode(NODE_TYPE(ConstantFloat4)), x(x), y(y), z(z), a(a)
        {}

        ConstantFloat4::ConstantFloat4(const Vector4D& value) : ConstantFloat4(value.x, value.y, value.z, value.a)
        {}

        node_compiler(ConstantFloat4, { return Strings::format("vec4({}, {}, {}, {})", x, y, z, a); });

        std::string ConstantFloat4::get_type()
        {
            return "vec4";
        }

        //////////////////////////////// Min IMPLEMENTATION ////////////////////////////////

        Min::Min(MaterialNode* first, MaterialNode* second)
            : MaterialNode(NODE_TYPE(Min)), _M_value1(first), _M_value2(second)
        {
            _M_link_to(first);
            _M_link_to(second);
        }

        INPUT_SLOT(Min, value1);
        INPUT_SLOT(Min, value2);

        node_compiler(Min, { return Strings::format("min({}, {})", gen_code(_M_value1), gen_code(_M_value2)); });

        std::string Min::get_type()
        {
            std::string types[2] = {(_M_value1 ? _M_value1->get_type() : ""), (_M_value1 ? _M_value1->get_type() : "")};
            int result[2] = {get_type_priority(types[0]), get_type_priority(types[1])};
            if (result[0] > result[1])
                return types[0];
            return types[1];
        }

        //////////////////////////////// Max IMPLEMENTATION ////////////////////////////////

        Max::Max(MaterialNode* first, MaterialNode* second)
            : MaterialNode(NODE_TYPE(Max)), _M_value1(first), _M_value2(second)
        {
            _M_link_to(first);
            _M_link_to(second);
        }


        INPUT_SLOT(Max, value1);
        INPUT_SLOT(Max, value2);

        node_compiler(Max, { return Strings::format("max({}, {})", gen_code(_M_value1), gen_code(_M_value2)); });

        std::string Max::get_type()
        {
            std::string types[2] = {(_M_value1 ? _M_value1->get_type() : ""), (_M_value1 ? _M_value1->get_type() : "")};
            int result[2] = {get_type_priority(types[0]), get_type_priority(types[1])};
            if (result[0] > result[1])
                return types[0];
            return types[1];
        }

        //////////////////////////////// VectorProperty IMPLEMENTATION ////////////////////////////////

        VectorProperty::VectorProperty(MaterialNode* vector, const std::string& prop)
            : MaterialNode(NODE_TYPE(VectorProperty)), _M_vector(vector), _M_property(prop)
        {
            _M_link_to(vector);
        }


        node_compiler(VectorProperty, { return Strings::format("{}.{}", gen_code(_M_vector), _M_property); });

        std::string VectorProperty::get_type()
        {
            return "float";
        }

        //////////////////////////////// Texture IMPLEMENTATION ////////////////////////////////

        Texture::Texture(Engine::Texture* texture) : MaterialNode(NODE_TYPE(Texture)), _M_texture(texture)
        {}

        INPUT_SLOT(Texture, UV);

        const Engine::Texture* Texture::texture() const
        {
            return _M_texture;
        }

        Texture& Texture::texture(Engine::Texture* value)
        {
            this->_M_texture = value;
            return *this;
        }

        node_compiler(Texture, {
            if (_M_uniform_index == Constants::index_none)
            {
                _M_uniform_index = shader_data.uniforms.size();
                std::string name = Strings::format("_uniform_{}", _M_uniform_index);
                shader_data.uniforms.push_back({name, Strings::format("uniform sampler2D {};", name)});
            }

            bool need_delete_uv = false;

            if ((need_delete_uv = (_M_UV == nullptr)))
                _M_UV = Object::new_instance<MaterialNodes::UV>();

            std::string result =
                    Strings::format("texture({}, {})", shader_data.uniforms[_M_uniform_index].first, gen_code(_M_UV));

            if (need_delete_uv)
            {
                _M_UV->mark_for_delete();
                _M_UV = nullptr;
            }

            return result;
        });

        std::string Texture::get_type()
        {
            return "vec4";
        }

        //////////////////////////////// UV IMPLEMENTATION ////////////////////////////////

        UV::UV() : MaterialNode(NODE_TYPE(UV))
        {}


        node_compiler(UV, {
            std::string result = "UV";
            for (const auto& ell : shader_data.in_params)
                if (ell.first == result)
                    return result;
            shader_data.in_params.push_back({"UV", "vec2 UV;"});
            return result;
        });

        std::string UV::get_type()
        {
            return "vec2";
        }

        //////////////////////////////// FragCoord IMPLEMENTATION ////////////////////////////////

        FragCoord::FragCoord() : MaterialNode(NODE_TYPE(FragCoord))
        {}

        node_compiler(FragCoord, {
            std::string result = "FragCoord";
            for (const auto& ell : shader_data.in_params)
                if (ell.first == result)
                    return result;
            shader_data.in_params.push_back({result, Strings::format("vec3 {};", result)});
            return result;
        });

        std::string FragCoord::get_type()
        {
            return "vec3";
        }

        //////////////////////////////// Sum IMPLEMENTATION ////////////////////////////////

        Sum::Sum(MaterialNode* first, MaterialNode* second)
            : MaterialNode(NODE_TYPE(Sum)), _M_value1(first), _M_value2(second)
        {
            _M_link_to(first);
            _M_link_to(second);
        }


        INPUT_SLOT(Sum, value1);
        INPUT_SLOT(Sum, value2);


        node_compiler(Sum, { return Strings::format("({} + {})", gen_code(_M_value1), gen_code(_M_value2)); });

        std::string Sum::get_type()
        {
            std::string types[2] = {(_M_value1 ? _M_value1->get_type() : ""), (_M_value1 ? _M_value1->get_type() : "")};
            int result[2] = {get_type_priority(types[0]), get_type_priority(types[1])};
            if (result[0] > result[1])
                return types[0];
            return types[1];
        }


        //////////////////////////////// Diff IMPLEMENTATION ////////////////////////////////

        Diff::Diff(MaterialNode* first, MaterialNode* second)
            : MaterialNode(NODE_TYPE(Diff)), _M_value1(first), _M_value2(second)
        {
            _M_link_to(first);
            _M_link_to(second);
        }


        INPUT_SLOT(Diff, value1);
        INPUT_SLOT(Diff, value2);

        node_compiler(Diff, { return Strings::format("({} - {})", gen_code(_M_value1), gen_code(_M_value2)); });

        std::string Diff::get_type()
        {
            std::string types[2] = {(_M_value1 ? _M_value1->get_type() : ""), (_M_value1 ? _M_value1->get_type() : "")};
            int result[2] = {get_type_priority(types[0]), get_type_priority(types[1])};
            if (result[0] > result[1])
                return types[0];
            return types[1];
        }

        //////////////////////////////// Multiply IMPLEMENTATION ////////////////////////////////

        Multiply::Multiply(MaterialNode* first, MaterialNode* second)
            : MaterialNode(NODE_TYPE(Multiply)), _M_value1(first), _M_value2(second)
        {
            _M_link_to(first);
            _M_link_to(second);
        }


        INPUT_SLOT(Multiply, value1);
        INPUT_SLOT(Multiply, value2);

        node_compiler(Multiply, { return Strings::format("({} - {})", gen_code(_M_value1), gen_code(_M_value2)); });

        std::string Multiply::get_type()
        {
            std::string types[2] = {(_M_value1 ? _M_value1->get_type() : ""), (_M_value1 ? _M_value1->get_type() : "")};
            int result[2] = {get_type_priority(types[0]), get_type_priority(types[1])};
            if (result[0] > result[1])
                return types[0];
            return types[1];
        }

        //////////////////////////////// Division IMPLEMENTATION ////////////////////////////////

        Division::Division(MaterialNode* first, MaterialNode* second)
            : MaterialNode(NODE_TYPE(Division)), _M_value1(first), _M_value2(second)
        {
            _M_link_to(first);
            _M_link_to(second);
        }

        INPUT_SLOT(Division, value1);
        INPUT_SLOT(Division, value2);

        node_compiler(Division, { return Strings::format("({} / {})", gen_code(_M_value1), gen_code(_M_value2)); });

        std::string Division::get_type()
        {
            std::string types[2] = {(_M_value1 ? _M_value1->get_type() : ""), (_M_value1 ? _M_value1->get_type() : "")};
            int result[2] = {get_type_priority(types[0]), get_type_priority(types[1])};
            if (result[0] > result[1])
                return types[0];
            return types[1];
        }

        //////////////////////////////// Pow IMPLEMENTATION ////////////////////////////////

        Pow::Pow(MaterialNode* first, MaterialNode* second)
            : MaterialNode(NODE_TYPE(Pow)), _M_value1(first), _M_value2(second)
        {
            _M_link_to(first);
            _M_link_to(second);
        }


        INPUT_SLOT(Pow, value1);
        INPUT_SLOT(Pow, value2);

        node_compiler(Pow, { return Strings::format("pow({}, {})", gen_code(_M_value1), gen_code(_M_value2)); });


        std::string Pow::get_type()
        {
            if (_M_value1)
                return _M_value1->get_type();
            return "";
        }
        //////////////////////////////// ConstantInt IMPLEMENTATION ////////////////////////////////

        ConstantInt::ConstantInt(int x) : MaterialNode(NODE_TYPE(ConstantInt)), value(x)
        {}

        node_compiler(ConstantInt, { return Strings::format("{}", value); });

        std::string ConstantInt::get_type()
        {
            return "int";
        }


        //////////////////////////////// ConstantFloat2 IMPLEMENTATION ////////////////////////////////

        ConstantInt2::ConstantInt2(int x, int y) : MaterialNode(NODE_TYPE(ConstantInt2)), x(x), y(y)
        {}


        ConstantInt2::ConstantInt2(const IntVector2D& value) : ConstantInt2(value.x, value.y)
        {}

        node_compiler(ConstantInt2, { return Strings::format("ivec2({}, {})", x, y); });

        std::string ConstantInt2::get_type()
        {
            return "ivec2";
        }

        //////////////////////////////// ConstantInt3 IMPLEMENTATION ////////////////////////////////


        ConstantInt3::ConstantInt3(int x, int y, int z) : MaterialNode(NODE_TYPE(ConstantInt3)), x(x), y(y), z(z)
        {}


        ConstantInt3::ConstantInt3(const IntVector3D& value) : ConstantInt3(value.x, value.y, value.z)
        {}

        node_compiler(ConstantInt3, { return Strings::format("ivec3({}, {}, {})", x, y, z); });

        std::string ConstantInt3::get_type()
        {
            return "ivec3";
        }

        //////////////////////////////// ConstantInt4 IMPLEMENTATION ////////////////////////////////

        ConstantInt4::ConstantInt4(int x, int y, int z, int a)
            : MaterialNode(NODE_TYPE(ConstantInt4)), x(x), y(y), z(z), a(a)
        {}

        ConstantInt4::ConstantInt4(const IntVector4D& value) : ConstantInt4(value.x, value.y, value.z, value.a)
        {}

        node_compiler(ConstantInt4, { return Strings::format("ivec4({}, {}, {}, {})", x, y, z, a); });

        std::string ConstantInt4::get_type()
        {
            return "ivec4";
        }

        //////////////////////////////// Sin IMPLEMENTATION ////////////////////////////////

        Sin::Sin(MaterialNode* value) : MaterialNode(NODE_TYPE(Sin)), _M_value(value)
        {
            _M_link_to(value);
        }

        INPUT_SLOT(Sin, value);

        node_compiler(Sin, { return Strings::format("sin({})", gen_code(_M_value)); });

        std::string Sin::get_type()
        {
            if (_M_value)
                return _M_value->get_type();
            return "";
        }

        //////////////////////////////// Cos IMPLEMENTATION ////////////////////////////////

        Cos::Cos(MaterialNode* value) : MaterialNode(NODE_TYPE(Cos)), _M_value(value)
        {
            _M_link_to(value);
        }

        INPUT_SLOT(Cos, value);

        node_compiler(Cos, { return Strings::format("cos({})", gen_code(_M_value)); });

        std::string Cos::get_type()
        {
            if (_M_value)
                return _M_value->get_type();
            return "";
        }

        //////////////////////////////// Tan IMPLEMENTATION ////////////////////////////////

        Tan::Tan(MaterialNode* value) : MaterialNode(NODE_TYPE(Tan)), _M_value(value)
        {
            _M_link_to(value);
        }

        INPUT_SLOT(Tan, value);

        node_compiler(Tan, { return Strings::format("tan({})", gen_code(_M_value)); });

        std::string Tan::get_type()
        {
            if (_M_value)
                return _M_value->get_type();
            return "";
        }

        //////////////////////////////// Normalize IMPLEMENTATION ////////////////////////////////

        Normalize::Normalize(MaterialNode* value) : MaterialNode(NODE_TYPE(Normalize)), _M_value(value)
        {
            _M_link_to(value);
        }

        INPUT_SLOT(Normalize, value);

        node_compiler(Normalize, { return Strings::format("normalize({})", gen_code(_M_value)); });

        std::string Normalize::get_type()
        {
            if (_M_value)
                return _M_value->get_type();
            return "";
        }
    }// namespace MaterialNodes


    //////////////////////////////// MATERIAL IMPLEMENTATION ////////////////////////////////


    PriorityIndex Material::priority() const
    {
        return _M_priority;
    }

    Material& Material::priority(PriorityIndex index)
    {
        _M_priority = index;
        return *this;
    }

    Material& Material::apply()
    {
        return *this;
    }
}// namespace Engine
