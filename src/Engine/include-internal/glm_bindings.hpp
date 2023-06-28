#pragma once
#include <Core/engine_lua.hpp>
#include <Core/engine_types.hpp>
#include <Core/engine_loading_controllers.hpp>

namespace Engine
{
    template<typename Type>
    String glm_to_string(const Type& object)
    {
        std::stringstream stream;
        stream << object;
        return stream.str();
    }

    template<typename Instance>
    static decltype(auto) index_function(Instance& instance, uint_t index)
    {
        return instance[index];
    }


#define OPERATOR_CREATOR(name, op)                                                                                     \
    template<typename Instance, typename Value>                                                                        \
    auto create_##name##_overator()                                                                                    \
    {                                                                                                                  \
        if constexpr (std::is_same_v<Quaternion, Instance>)                                                            \
        {                                                                                                              \
            return [](const Instance& v1, const Instance& v2) -> Instance { return v1 op v2; };                        \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            return sol::overload([](const Instance& v1, const Instance& v2) -> Instance { return v1 op v2; },          \
                                 [](const Instance& v1, Value f) -> Instance { return v1 op f; },                      \
                                 [](Value f, const Instance& v1) -> Instance { return f op v1; });                     \
        }                                                                                                              \
    }

    OPERATOR_CREATOR(plus, +);
    OPERATOR_CREATOR(minus, -);
    OPERATOR_CREATOR(mult, *);
    OPERATOR_CREATOR(div, /);


    template<typename Instance>
    static void initialize_arithmetic_functions(Lua::Class<Instance>& lua_class)
    {
        lua_class.set(Lua::meta_function::index,
                      Lua::overload(index_function<Instance>, index_function<const Instance>));
        lua_class.set(Lua::meta_function::addition, create_plus_overator<Instance, typename Instance::value_type>());
        lua_class.set(Lua::meta_function::subtraction,
                      create_minus_overator<Instance, typename Instance::value_type>());
        lua_class.set(Lua::meta_function::multiplication,
                      create_mult_overator<Instance, typename Instance::value_type>());
        if constexpr (!std::is_same_v<Quaternion, Instance>)
        {
            lua_class.set(Lua::meta_function::division, create_div_overator<Instance, typename Instance::value_type>());

            lua_class.set(Lua::meta_function::construct, Lua::constructors<Instance(), Instance(const Instance&),
                                                                           Instance(typename Instance::value_type)>());
        }
        else
        {
            lua_class.set(Lua::meta_function::construct, Lua::constructors<Instance(), Instance(const Instance&)>());
        }
    }

    template<typename VectorType>
    static void initialize_vector(const String& name)
    {
        Lua::Class<VectorType> vector_class = Lua::Interpretter::lua_class_of<VectorType>(name);
        initialize_arithmetic_functions(vector_class);

        vector_class.set(Lua::meta_function::to_string, &glm_to_string<VectorType>);
        vector_class.set("length", &VectorType::length);
        vector_class.set("x", &VectorType::x);


        if constexpr (!std::is_same_v<VectorType, Quaternion>)
        {
            vector_class.set("s", &VectorType::s);
            vector_class.set("r", &VectorType::r);
        }

        constexpr size_t lenght = VectorType().length();

        if constexpr (lenght > 1)
        {
            vector_class.set("y", &VectorType::y);
            if constexpr (!std::is_same_v<VectorType, Quaternion>)
            {
                vector_class.set("g", &VectorType::g);
                vector_class.set("t", &VectorType::t);
            }
        }

        if constexpr (lenght > 2)
        {
            vector_class.set("z", &VectorType::z);
            if constexpr (!std::is_same_v<VectorType, Quaternion>)
            {
                vector_class.set("b", &VectorType::b);
                vector_class.set("p", &VectorType::p);
            }
        }

        if constexpr (lenght > 3)
        {
            vector_class.set("w", &VectorType::w);
            if constexpr (!std::is_same_v<VectorType, Quaternion>)
            {
                vector_class.set("a", &VectorType::a);
                vector_class.set("q", &VectorType::q);
            }
        }
    }


    template<typename Matrix>
    static void initialize_matrix(const String& name)
    {
        Lua::Class<Matrix> matrix_class = Lua::Interpretter::lua_class_of<Matrix>(name);
        initialize_arithmetic_functions(matrix_class);

        matrix_class.set(Lua::meta_function::to_string, &glm_to_string<Matrix>);
        matrix_class.set("length", &Matrix::length);
    }

}// namespace Engine
