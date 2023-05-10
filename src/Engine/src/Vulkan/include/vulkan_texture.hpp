#pragma once
#include <Core/render_types.hpp>
#include <Core/texture_types.hpp>
#include <vulkan_object.hpp>


namespace Engine
{

    struct VulkanTextureState {
        vk::Extent2D size;
        MipMapLevel mipmap_count;
        vk::Format format;
        vk::Filter min_filter;
        vk::Filter mag_filter;
        vk::SampleCountFlagBits sample_count = vk::SampleCountFlagBits::e1;
        vk::SamplerMipmapMode sampler_mipmap_mode;
        vk::SamplerAddressMode wrap_s;
        vk::SamplerAddressMode wrap_t;
        vk::SamplerAddressMode wrap_r;
        float mip_lod_bias                = 0.0;
        byte anisotropy_enable : 1        = 0;
        float anisotropy                  = 1.0;
        byte compare_enable : 1           = 0;
        LodLevel min_lod                  = -1000.0;
        LodLevel max_lod                  = 1000.0;
        byte unnormalized_coordinates : 1 = 0;
        vk::CompareOp compare_func;
        vk::ComponentMapping swizzle;
        byte pixel_component_count = 0;
        MipMapLevel base_mip_level;
        VulkanTextureState& init(const TextureCreateInfo& info);
    };

    struct VulkanTexture : VulkanObject {
        vk::Image _M_image;
        vk::DeviceMemory _M_image_memory;
        vk::ImageView _M_image_view;
        vk::Sampler _M_texture_sampler;
        uint_t _M_layer_count = 1;
        vk::ImageAspectFlags _M_image_aspect;
        vk::ImageViewType _M_image_type;

        VulkanTextureState state;


        VulkanTexture();
        VulkanTexture& init(const TextureCreateInfo& params, TextureType type);
        vk::Format format();
        vk::Offset2D get_mip_size(MipMapLevel level);
        vk::ImageView get_image_view(const vk::ImageSubresourceRange& range);
        bool is_depth_stencil_image();


        VulkanTexture& read_texture_2D_data(Vector<byte>& data, MipMapLevel);
        VulkanTexture& swizzle(const SwizzleRGBA& swizzle);
        SwizzleRGBA swizzle();
        VulkanTexture& min_filter(TextureFilter filter);
        VulkanTexture& mag_filter(TextureFilter filter);
        TextureFilter min_filter();
        TextureFilter mag_filter();
        VulkanTexture& wrap_s(WrapValue value);
        VulkanTexture& wrap_t(WrapValue value);
        VulkanTexture& wrap_r(WrapValue value);
        WrapValue wrap_s();
        WrapValue wrap_t();
        WrapValue wrap_r();
        VulkanTexture& compare_func(CompareFunc func);
        CompareFunc compare_func();
        VulkanTexture& compare_mode(CompareMode mode);
        CompareMode compare_mode();
        VulkanTexture& base_level(MipMapLevel level);
        VulkanTexture& min_lod_level(LodLevel level);
        VulkanTexture& max_lod_level(LodLevel level);
        VulkanTexture& generate_mipmap();
        Size2D size(MipMapLevel level);


        SamplerMipmapMode sample_mipmap_mode_texture();
        VulkanTexture& sample_mipmap_mode_texture(SamplerMipmapMode mode);
        VulkanTexture& lod_bias_texture(LodBias bias);
        VulkanTexture& unnormalized_coordinates_texture(bool flag);

        VulkanTexture& update_texture_2D(const Size2D& size, const Offset2D& offset, MipMapLevel level, uint_t layer,
                                         const void* data);
        PixelType pixel_type();
        VulkanTexture& anisotropic_value(float value);
        bool can_use_color_as_color_attachment();
        static vk::Format parse_format(PixelType type, PixelComponentType component);
        ~VulkanTexture();

    private:
        VulkanTexture& create_image();
        VulkanTexture& create_image_view();
        VulkanTexture& create_texture_sampler();
        vk::ImageViewType image_view_type();
        vk::ImageSubresourceRange subresource_range(MipMapLevel base);
        uint_t pixel_type_size();
        VulkanTexture& clear();
    };
}// namespace Engine
