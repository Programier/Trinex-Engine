#pragma once
#include <Image/image.hpp>
#include <string>

namespace Engine
{
    class Skybox
    {
        unsigned int _M_ID = 0;
        Image _M_images[6];


        void update_id();

        void delete_skybox();

    public:
        Skybox();
        Skybox(const std::string& filename, const bool& invert = false);

        //! @brief {right, left, top, bottom, front, back}
        Skybox(const std::vector<std::string>& filenames, const bool& invert = false);
        Skybox(const Skybox&);

        Skybox& operator=(const Skybox&);


        Skybox& load(const std::string& filename, const bool& invert = false);
        //! @brief {right, left, top, bottom, front, back}
        Skybox& load(const std::vector<std::string>& filenames, const bool& invert = false);
        Skybox& draw();
        ~Skybox();
    };

}// namespace Engine