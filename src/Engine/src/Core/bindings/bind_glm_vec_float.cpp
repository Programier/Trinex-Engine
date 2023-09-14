#include "bind_glm.hpp"
#include "bind_glm_functions.hpp"
#include <Core/engine_loading_controllers.hpp>


namespace Engine
{

    implement_vector_wrapper(Vector1D);
    implement_vector_wrapper(Vector2D);
    implement_vector_wrapper(Vector3D);
    implement_vector_wrapper(Vector4D);


    static void bind_functions()
    {
        String vtype = "float";
        bind_wrapped_functions<Vector1DWrapper, Vector1D, All ^ Cross>("Engine::Vector1D", vtype);
        bind_wrapped_functions<Vector2DWrapper, Vector2D, All ^ Cross>("Engine::Vector2D", vtype);
        bind_wrapped_functions<Vector3DWrapper, Vector3D, All>("Engine::Vector3D", vtype);
        bind_wrapped_functions<Vector4DWrapper, Vector4D, All ^ Cross>("Engine::Vector4D", vtype);
    }

    static void on_init()
    {
        String prop_type       = "float";
        String class_name      = "Engine::Vector";
        const char* const_type = "float";
        const char* ref_type   = "float&";
        using RegistryType1    = Vector1DWrapper;
        using RegistryType2    = Vector2DWrapper;
        using RegistryType3    = Vector3DWrapper;
        using RegistryType4    = Vector4DWrapper;
        {
            using ConstType = RegistryType1::value_type;
            using RefType   = RegistryType1::value_type&;
            ScriptClassRegistrar registrar(class_name + "1D", info_of<RegistryType1>());
            bind_glm_behaviours<RegistryType1>(registrar, prop_type);
            bind_vec1_props<RegistryType1>(registrar, prop_type);
            bind_glm_operators<RegistryType1>(registrar, prop_type);
            bind_index_op<RegistryType1, ConstType, RefType>(registrar, const_type, ref_type);
        }
        {
            using ConstType = RegistryType2::value_type;
            using RefType   = RegistryType2::value_type&;
            ScriptClassRegistrar registrar(class_name + "2D", info_of<RegistryType2>());
            bind_glm_behaviours<RegistryType2>(registrar, prop_type);
            bind_vec2_props<RegistryType2>(registrar, prop_type);
            bind_glm_operators<RegistryType2>(registrar, prop_type);
            bind_index_op<RegistryType2, ConstType, RefType>(registrar, const_type, ref_type);
        }
        {
            using ConstType = RegistryType3::value_type;
            using RefType   = RegistryType3::value_type&;
            ScriptClassRegistrar registrar(class_name + "3D", info_of<RegistryType3>());
            bind_glm_behaviours<RegistryType3>(registrar, prop_type);
            bind_vec3_props<RegistryType3>(registrar, prop_type);
            bind_glm_operators<RegistryType3>(registrar, prop_type);
            bind_index_op<RegistryType3, ConstType, RefType>(registrar, const_type, ref_type);
        }
        {
            using ConstType = RegistryType3::value_type;
            using RefType   = RegistryType3::value_type&;
            ScriptClassRegistrar registrar(class_name + "4D", info_of<RegistryType4>());
            bind_glm_behaviours<RegistryType4>(registrar, prop_type);
            bind_vec4_props<RegistryType4>(registrar, prop_type);
            bind_glm_operators<RegistryType4>(registrar, prop_type);
            bind_index_op<RegistryType4, ConstType, RefType>(registrar, const_type, ref_type);
        }

        bind_functions();
    }

    static InitializeController controller(on_init, "Bind Engine::Vector");
}// namespace Engine
