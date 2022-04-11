#include <BasicFunctional/basic_functional.hpp>
#include <Graphics/object.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <thread>


namespace Engine
{
    static void rotate_part_of_hitboxes(IHitBox* data, std::size_t size, const glm::vec3* rotation)
    {
        for (std::size_t i = 0; i < size; i++) data[i].rotation() += *rotation;
    }

    static void rotate_hitboxes(std::vector<IHitBox>& hitboxes, const glm::vec3& rotation)
    {
        std::size_t size = hitboxes.size();
        std::vector<std::thread> threads;
        std::size_t block = size / processor_count;
        std::size_t last_block = size - (block * processor_count - 1);
        auto data = hitboxes.data();
        for (unsigned int i = 0; i < processor_count - 1; i++)
            threads.emplace_back(rotate_part_of_hitboxes, data + i * block, block, &rotation);
        rotate_part_of_hitboxes(data + (processor_count - 1) * block, last_block, &rotation);
        for (auto& thread : threads) thread.join();
    }

    static void move_part_of_hitboxes(IHitBox* data, std::size_t size, const glm::vec3* pos)
    {
        for (std::size_t i = 0; i < size; i++) data[i].position() += *pos;
    }

    static void move_hitboxes(std::vector<IHitBox>& hitboxes, const glm::vec3& pos)
    {
        std::size_t size = hitboxes.size();
        std::vector<std::thread> threads;
        std::size_t block = size / processor_count;
        std::size_t last_block = size - (block * processor_count - 1);
        auto data = hitboxes.data();
        for (unsigned int i = 0; i < processor_count - 1; i++)
            threads.emplace_back(move_part_of_hitboxes, data + i * block, block, &pos);
        move_part_of_hitboxes(data + (processor_count - 1) * block, last_block, &pos);
        for (auto& thread : threads) thread.join();
    }

    Object& Object::update_model()
    {
        _M_model = glm::scale(glm::mat4(1.0f), _M_rotation);
        _M_model = glm::translate(_M_model, _M_position);
        for (int i = 0; i < 3; i++)
        {
            glm::vec3 vector = {0.f, 0.f, 0.f};
            vector[i] = 1.0f;
            _M_model = glm::rotate(_M_model, _M_rotation[i], vector);
        }
        return *this;
    }

    Object::Object(const Object&) = default;
    Object::Object(const glm::vec3& _position, const glm::vec3& _rotation, const Scale& _scale)
        : _M_position(_position), _M_rotation(_rotation), _M_scale(_scale)
    {
        update_model();
    }

    Object& Object::operator=(const Object&) = default;


    Object& Object::rotate(float x, float y, float z, const bool& add_value)
    {
        return rotate({x, y, z}, add_value);
    }

    Object& Object::rotate(const glm::vec3& rotation, const bool& add_value)
    {
        auto R = add_value ? rotation : (-_M_rotation) + rotation;
        rotate_hitboxes(_M_hitboxes, R);
        for (int i = 0; i < 3; i++)
        {
            glm::vec3 axis;
            axis[i] = 1.0f;
            _M_model = glm::rotate(_M_model, R[i], axis);
        }
        _M_rotation += R;
        return *this;
    }

    Object& Object::rotate(const float& angle, const glm::vec3& axis)
    {
        _M_model = glm::rotate(_M_model, angle, axis);
        glm::vec3 tmp_rotation = get_rotation_from_matrix(_M_model);
        rotate_hitboxes(_M_hitboxes, tmp_rotation - _M_rotation);
        _M_rotation = tmp_rotation;
        return *this;
    }

    const glm::vec3& Object::rotation() const
    {
        return _M_rotation;
    }

    Object& Object::move(const float& right, const float& up, const float& forward, const glm::vec3& right_a,
                         const glm::vec3& up_a, const glm::vec3& front_a)
    {
        auto tmp = _M_position;
        _M_position += front_a * forward;
        _M_position += right_a * right;
        _M_position += up_a * up;
        auto diff = _M_position - tmp;
        move_hitboxes(_M_hitboxes, diff);
        _M_model = glm::translate(_M_model, diff);
        return *this;
    }
    Object& Object::move(const glm::vec3& move_vector, const glm::vec3& right_a, const glm::vec3& up_a,
                         const glm::vec3& front_a)
    {
        return move(move_vector.x, move_vector.y, move_vector.z, right_a, up_a, front_a);
    }

    const Scale& Object::scale() const
    {
        return _M_scale;
    }

    Object& Object::scale(const Scale& _scale, const bool& from_original_size)
    {
        glm::vec3 tmp = {1.f, 1.f, 1.f};
        if (from_original_size)
            tmp /= _M_scale;
        _M_model = glm::scale(_M_model, tmp * _scale);
        _M_scale = from_original_size ? _scale : _M_scale * _scale;
        return *this;
    }

    const glm::mat4& Object::model_matrix() const
    {
        return _M_model;
    }

    const glm::vec3& Object::coords() const
    {
        return _M_position;
    }

    Object& Object::coords(const glm::vec3& _coords)
    {
        auto diff = _coords - _M_position;
        move_hitboxes(_M_hitboxes, diff);
        _M_model = glm::translate(_M_model, diff);
        _M_position = _coords;
        return *this;
    }

    const std::vector<IHitBox>& Object::hitboxes() const
    {
        return _M_hitboxes;
    }

    std::vector<IHitBox>& Object::hitboxes()
    {
        return _M_hitboxes;
    }
}// namespace Engine
