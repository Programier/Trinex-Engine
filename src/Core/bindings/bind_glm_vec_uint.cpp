#include "bind_glm.hpp"
#include "bind_glm_functions.hpp"
#include <Core/engine_loading_controllers.hpp>


namespace Engine
{
    implement_vector_wrapper(UIntVector1D);
    implement_vector_wrapper(UIntVector2D);
    implement_vector_wrapper(UIntVector3D);
    implement_vector_wrapper(UIntVector4D);


    static void bind_functions()
    {
        String vtype = "uint";

        bind_wrapped_functions<UIntVector1DWrapper, UIntVector1D, All ^ Length ^ Cross ^ Normalize ^ Dot>("Engine::UIntVector1D",
                                                                                                          vtype);
        bind_wrapped_functions<UIntVector2DWrapper, UIntVector2D, All ^ Length ^ Cross ^ Normalize ^ Dot>("Engine::UIntVector2D",
                                                                                                          vtype);
        bind_wrapped_functions<UIntVector3DWrapper, UIntVector3D, All ^ Length ^ Cross ^ Normalize ^ Dot>("Engine::UIntVector3D",
                                                                                                          vtype);
        bind_wrapped_functions<UIntVector4DWrapper, UIntVector4D, All ^ Length ^ Cross ^ Normalize ^ Dot>("Engine::UIntVector4D",
                                                                                                          vtype);
    }

    static void on_init()
    {
        String prop_type       = "uint";
        String class_name      = "Engine::UIntVector";
        const char* const_type = "uint";
        const char* ref_type   = "uint&";
        using RegistryType1    = UIntVector1DWrapper;
        using RegistryType2    = UIntVector2DWrapper;
        using RegistryType3    = UIntVector3DWrapper;
        using RegistryType4    = UIntVector4DWrapper;
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

    static ReflectionInitializeController controller(on_init, "Engine::UIntVector");
}// namespace Engine
