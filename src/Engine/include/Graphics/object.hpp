#pragma once
#include <Graphics/hitbox.hpp>
#include <Graphics/mesh.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Engine
{
    typedef glm::vec3 ObjectPosition;
    typedef glm::vec3 Scale;
    class Object
    {
    protected:
        std::vector<IHitBox> _M_hitboxes;

    public:
        // Methods
        Object(const Object&);
        Object(const glm::vec3& position = {0.f, 0.f, 0.f}, const glm::vec3& rotation = {0.f, 0.f, 0.f},
               const Scale& scale = {1.f, 1.f, 1.f});
        Object& operator=(const Object&);
        const std::vector<IHitBox>& hitboxes() const;
        std::vector<IHitBox>& hitboxes();
    };
}// namespace Engine
