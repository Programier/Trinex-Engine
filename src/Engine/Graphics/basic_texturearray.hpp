#pragma once
#include <list>
#include <vector>
#include "../Image/image.hpp"
#include <glm/glm.hpp>





namespace Engine {
    namespace basic_texturearray {
        unsigned int gen_texture_array(const std::vector<Image *>& images, const glm::vec2& max_size);
        unsigned int gen_texture_array(const std::list<Image *>& images, const glm::vec2& max_size);
        unsigned int gen_texture_array(const std::vector<Image>& images, const glm::vec2& max_size);
        unsigned int gen_texture_array(const std::list<Image>& images, const glm::vec2& max_size);
    }
}
