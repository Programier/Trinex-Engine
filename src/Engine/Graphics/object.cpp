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


    Object::Object(const Object&) = default;
    Object::Object(const glm::vec3& _position, const glm::vec3& _rotation, const Scale& _scale)
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
