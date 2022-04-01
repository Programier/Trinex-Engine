#pragma once
#include <Image/image.hpp>
#include <glm/glm.hpp>
#include <list>
#include <vector>


namespace Engine
{
    namespace basic_texturearray
    {
        unsigned int gen_texture_array(const std::vector<Image*>& images, const glm::vec2& max_size);
        unsigned int gen_texture_array(const std::list<Image*>& images, const glm::vec2& max_size);
        unsigned int gen_texture_array(const std::vector<Image>& images, const glm::vec2& max_size);
        unsigned int gen_texture_array(const std::list<Image>& images, const glm::vec2& max_size);
    }// namespace basic_texturearray
}// namespace Engine
