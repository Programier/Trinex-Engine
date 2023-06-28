#include <glm_bindings.hpp>

namespace Engine
{
    static void on_init()
    {
        initialize_vector<BoolVector1D>("Engine::BoolVector1D");
        initialize_vector<BoolVector2D>("Engine::BoolVector2D");
        initialize_vector<BoolVector3D>("Engine::BoolVector3D");
        initialize_vector<BoolVector4D>("Engine::BoolVector4D");
    }

    static InitializeController initializer(on_init);
}// namespace Engine
