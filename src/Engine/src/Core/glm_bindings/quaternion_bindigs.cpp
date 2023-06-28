#include <glm_bindings.hpp>

namespace Engine
{
    static void on_init()
    {
        initialize_vector<Quaternion>("Engine::Quaternion");
    }

    static InitializeController initializer(on_init);
}// namespace Engine
