#include <Core/class.hpp>
#include <Core/base_engine.hpp>
#include <Core/logger.hpp>
#include <Graphics/skybox.hpp>

static void init_mesh()
{}


namespace Engine
{
    implement_class(Skybox, Engine, 0);
    implement_default_initialize_class(Skybox);


    Skybox::Skybox()
    {}

    Skybox& Skybox::load(const Vector<String>& filenames, const bool& invert)
    {
        return *this;
    }

    Skybox& Skybox::load(const String& filename, const bool& invert)
    {
        init_mesh();
        Image img(filename, invert);

        if (img.empty())
        {
            info_log("Skybox", "Failed to load skybox\n");
            return *this;
        }


        int block_width  = img.width() / 4;
        int block_height = img.height() / 3;

        if (!TextureCubeMap::has_object())
        {
            format       = img.format();
            mipmap_count = 8;
            size         = Size2D(block_width, block_height);
            rhi_create();
        }

        Image m_images[6];

//        m_images[(byte) TextureCubeMapFace::Up] =
//                invert ? img.sub_image({block_width, block_height * 2}, {block_width * 2, block_height * 3})
//                       : img.sub_image({block_width, 0}, {block_width * 2, block_height});


//        m_images[(byte) TextureCubeMapFace::Down] =
//                invert ? img.sub_image({block_width, 0}, {block_width * 2, block_height})
//                       : img.sub_image({block_width, block_height * 2}, {block_width * 2, block_height * 3});

//        m_images[(byte) TextureCubeMapFace::Left] = img.sub_image({0, block_height}, {block_width, block_height * 2});
//        m_images[(byte) TextureCubeMapFace::Right] =
//                img.sub_image({block_width * 2, block_height}, {block_width * 3, block_height * 2});
//        m_images[(byte) TextureCubeMapFace::Back] =
//                img.sub_image({block_width * 1, block_height}, {block_width * 2, block_height * 2});
//        m_images[(byte) TextureCubeMapFace::Front] =
//                img.sub_image({block_width * 3, block_height}, {block_width * 4, block_height * 2});


        for (byte i = 0; i < 6; i++)
        {
            TextureCubeMap::update_data((TextureCubeMapFace) i, m_images[i].size(), {0, 0},
                                        (void*) m_images[i].data(), 0);
        }

        return *this;
    }

    Skybox& Skybox::draw()
    {
        throw std::runtime_error(not_implemented);
        return *this;
    }


    Skybox::Skybox(const String& filename, const bool& invert)
    {
        load(filename, invert);
    }

    Skybox::Skybox(const Vector<String>& filenames, const bool& invert)
    {
        load(filenames, invert);
    }
}// namespace Engine
