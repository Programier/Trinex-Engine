#include <BasicFunctional/basic_functional.hpp>
#include <Graphics/object.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <thread>


namespace Engine
{
    Object::Object(const Object&) = default;
    Object::Object(const glm::vec3& _position, const glm::vec3& _rotation, const glm::vec3& _scale)
    {}

    Object& Object::operator=(const Object&) = default;

    const std::vector<IHitBox>& Object::hitboxes() const
    {
        return _M_hitboxes;
    }

    std::vector<IHitBox>& Object::hitboxes()
    {
        return _M_hitboxes;
    }
}// namespace Engine
