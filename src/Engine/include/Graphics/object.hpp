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
        glm::vec3 _M_position = {0.f, 0.f, 0.f};
        glm::vec3 _M_rotation = {0.f, 0.f, 0.f};
        Scale _M_scale = {1.f, 1.f, 1.f};

        glm::mat4 _M_model = glm::mat4(1.0f);

        Object& update_model();

    public:
        // Methods
        Object(const Object&);
        Object(const glm::vec3& position = {0.f, 0.f, 0.f}, const glm::vec3& rotation = {0.f, 0.f, 0.f},
               const Scale& scale = {1.f, 1.f, 1.f});
        Object& operator=(const Object&);

        Object& rotate(float x, float y, float z, const bool& add_value = true);
        Object& rotate(const glm::vec3& rotation, const bool& add_value = true);
        Object& rotate(const float& angle, const glm::vec3& axis);
        const glm::vec3& rotation() const;
        Object& move(const float& right, const float& up, const float& forward, const glm::vec3& right_a,
                     const glm::vec3& up_a, const glm::vec3& front_a);
        Object& move(const glm::vec3& move_vector, const glm::vec3& right_a, const glm::vec3& up_a,
                     const glm::vec3& front_a);
        const Scale& scale() const;
        Object& scale(const Scale& scale, const bool& from_original_size = false);

        const glm::mat4& model_matrix() const;
        const glm::vec3& coords() const;
        Object& coords(const glm::vec3& coords);
        const std::vector<IHitBox>& hitboxes() const;
        std::vector<IHitBox>& hitboxes();
    };
}// namespace Engine
