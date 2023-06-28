#include <glm_bindings.hpp>

namespace Engine
{
    static void on_init()
    {
        initialize_vector<UIntVector1D>("Engine::UIntVector1D");
        initialize_vector<UIntVector2D>("Engine::UIntVector2D");
        initialize_vector<UIntVector3D>("Engine::UIntVector3D");
        initialize_vector<UIntVector4D>("Engine::UIntVector4D");
    }

    static InitializeController initializer(on_init);
}// namespace Engine
