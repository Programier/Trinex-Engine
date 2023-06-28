#include <glm_bindings.hpp>

namespace Engine
{
    static void on_init()
    {
        initialize_vector<IntVector1D>("Engine::IntVector1D");
        initialize_vector<IntVector2D>("Engine::IntVector2D");
        initialize_vector<IntVector3D>("Engine::IntVector3D");
        initialize_vector<IntVector4D>("Engine::IntVector4D");
    }

    static InitializeController initializer(on_init);
}// namespace Engine
