#include <glm_bindings.hpp>

namespace Engine
{

    template<typename Type>
    static decltype(auto) wrap_glm_normalize(const Type& v)
    {
        return glm::normalize(v);
    }

    template<typename Type>
    static decltype(auto) wrap_glm_length(const Type& v)
    {
        return glm::length(v);
    }

    template<typename Type>
    static decltype(auto) wrap_glm_dot(const Type& v1, const Type& v2)
    {
        return glm::dot(v1, v2);
    }

    template<typename Type>
    static decltype(auto) wrap_glm_cross(const Type& v1, const Type& v2)
    {
        return glm::cross(v1, v2);
    }

    template<typename Type>
    static decltype(auto) wrap_glm_min(const Type& v1, const Type& v2)
    {
        return glm::min(v1, v2);
    }

    template<typename Type>
    static decltype(auto) wrap_glm_max(const Type& v1, const Type& v2)
    {
        return glm::max(v1, v2);
    }

    template<typename Type, typename Type2 = float>
    static decltype(auto) wrap_glm_clamp(const Type& v1, const Type2& v2, const Type2& v3)
    {
        return glm::clamp(v1, v2, v3);
    }

    template<typename QuaternionType, typename... VectorTypes>
    static void initialize_vector_functions()
    {
        Lua::Namespace glm_namespace = Lua::Interpretter::namespace_of("glm::");

        glm_namespace.set_function(
                "normalize", Lua::overload(&wrap_glm_normalize<VectorTypes>..., &wrap_glm_normalize<QuaternionType>));

        glm_namespace.set_function("length",
                                   Lua::overload(&wrap_glm_length<VectorTypes>..., &wrap_glm_length<QuaternionType>));

        glm_namespace.set_function("dot", Lua::overload(&wrap_glm_dot<VectorTypes>..., &wrap_glm_dot<QuaternionType>));
        glm_namespace.set_function("cross", &wrap_glm_cross<Vector3D>);
        glm_namespace.set_function("min", Lua::overload(&wrap_glm_min<VectorTypes>...));
        glm_namespace.set_function("max", Lua::overload(&wrap_glm_max<VectorTypes>...));
        glm_namespace.set_function("clamp", Lua::overload(&wrap_glm_clamp<VectorTypes, VectorTypes>...,
                                                          &wrap_glm_clamp<VectorTypes, float>...));
    }

    static void on_init()
    {
        initialize_vector_functions<Quaternion, Vector1D, Vector2D, Vector3D, Vector4D>();
    }

    static InitializeController initializer(on_init);
}// namespace Engine
