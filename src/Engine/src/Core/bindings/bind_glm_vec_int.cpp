#include "bind_glm.hpp"
#include "bind_glm_functions.hpp"
#include <Core/engine_loading_controllers.hpp>


namespace Engine
{
    implement_vector_wrapper(IntVector1D);
    implement_vector_wrapper(IntVector2D);
    implement_vector_wrapper(IntVector3D);
    implement_vector_wrapper(IntVector4D);

    static void bind_functions()
    {
        String vtype = "int";

        bind_wrapped_functions<IntVector1DWrapper, IntVector1D, All ^ Length ^ Cross ^ Normalize ^ Dot>("Engine::IntVector1D",
                                                                                                        vtype);
        bind_wrapped_functions<IntVector2DWrapper, IntVector2D, All ^ Length ^ Cross ^ Normalize ^ Dot>("Engine::IntVector2D",
                                                                                                        vtype);
        bind_wrapped_functions<IntVector3DWrapper, IntVector3D, All ^ Length ^ Cross ^ Normalize ^ Dot>("Engine::IntVector3D",
                                                                                                        vtype);
        bind_wrapped_functions<IntVector4DWrapper, IntVector4D, All ^ Length ^ Cross ^ Normalize ^ Dot>("Engine::IntVector4D",
                                                                                                        vtype);
    }

    static void on_init()
    {
        String prop_type       = "int";
        String class_name      = "Engine::IntVector";
        const char* const_type = "int";
        const char* ref_type   = "int&";
        using RegistryType1    = IntVector1DWrapper;
        using RegistryType2    = IntVector2DWrapper;
        using RegistryType3    = IntVector3DWrapper;
        using RegistryType4    = IntVector4DWrapper;
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

    static ReflectionInitializeController controller(on_init, "Bind Engine::IntVector");
}// namespace Engine
