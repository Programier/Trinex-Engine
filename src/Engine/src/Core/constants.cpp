#include <Core/constants.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_lua.hpp>
#include <thread>

namespace Engine
{
    const unsigned int Constants::processor_count = std::thread::hardware_concurrency();
    const glm::vec3 Constants::OX(1.f, 0.f, 0.f);
    const glm::vec3 Constants::OY(0.f, 1.f, 0.f);
    const glm::vec3 Constants::OZ(0.f, 0.f, 1.f);
    const float Constants::PI                         = glm::pi<float>();
    const float Constants::E                          = glm::e<float>();
    const glm::mat4 Constants::identity_matrix        = Matrix4f(1.0f);
    const glm::mat4 Constants::zero_matrix            = Matrix4f();
    const glm::vec4 Constants::identity_vector        = Vector4D(1.f, 1.f, 1.f, 1.f);
    const glm::vec4 Constants::zero_vector            = Vector4D(0.f, 0.f, 0.f, 0.f);
    const float Constants::min_positive_float         = 0.000001f;
    const Vector3D Constants::min_positive_vector     = Vector3D(0.000001f);
    const ArrayIndex Constants::index_none            = ~0U;
    const size_t Constants::max_size                  = ~0U;
    const IntVector4D Constants::int_zero_vector      = IntVector4D(0);
    const IntVector4D Constants::int_identity_vector  = IntVector4D(1);
    const IntVector4D Constants::uint_zero_vector     = UIntVector4D(0);
    const IntVector4D Constants::uint_identity_vector = UIntVector4D(1);
    const String Constants::package_extention         = ".tpk";
    const String Constants::name_separator            = "::";

    static void on_init()
    {
        Lua::Namespace _namespace = Lua::Interpretter::namespace_of("Engine::Constants::");

#define set_value(x) _namespace.set(#x, &Constants::x)
        set_value(processor_count);
        set_value(OX);
        set_value(OY);
        set_value(OZ);
        set_value(PI);
        set_value(E);
        set_value(identity_matrix);
        set_value(zero_matrix);
        set_value(identity_vector);
        set_value(zero_vector);
        set_value(min_positive_float);
        set_value(min_positive_vector);
        set_value(index_none);
        set_value(max_size);
        set_value(int_zero_vector);
        set_value(int_identity_vector);
        set_value(uint_zero_vector);
        set_value(uint_identity_vector);
        set_value(package_extention);
        set_value(name_separator);
    }

    static InitializeController init(on_init);
}// namespace Engine
