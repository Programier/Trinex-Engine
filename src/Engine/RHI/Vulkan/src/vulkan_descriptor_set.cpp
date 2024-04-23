#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_descriptor_set.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_texture.hpp>

namespace Engine
{
    VulkanDescriptorSet& VulkanDescriptorSet::init(vk::DescriptorPool& pool, vk::DescriptorSetLayout* layout)
    {
        vk::DescriptorSetAllocateInfo info(pool, *layout);
        m_set = API->m_device.allocateDescriptorSets(info).front();
        return *this;
    }

    VulkanDescriptorSet& VulkanDescriptorSet::bind(vk::PipelineLayout& layout, BindingIndex set)
    {
        API->current_command_buffer().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, set, 1, &m_set, 0, nullptr);
        return *this;
    }

    VulkanDescriptorSet& VulkanDescriptorSet::bind_ssbo(struct VulkanSSBO* ssbo, BindingIndex index)
    {
        VulkanSSBO*& current_ssbo = m_ssbo[index];
        if (current_ssbo != ssbo)
        {
            vk::DescriptorBufferInfo buffer_info(ssbo->m_buffer.m_buffer, 0, ssbo->m_buffer.m_size);
            vk::WriteDescriptorSet write_descriptor(m_set, index, 0, vk::DescriptorType::eStorageBuffer, {}, buffer_info);
            API->m_device.updateDescriptorSets(write_descriptor, {});
            current_ssbo = ssbo;
        }
        return *this;
    }

    VulkanDescriptorSet& VulkanDescriptorSet::bind_uniform_buffer(const vk::Buffer& buffer, size_t offset, size_t size,
                                                                  BindingIndex index)
    {
        vk::DescriptorBufferInfo buffer_info(buffer, offset, size);
        vk::WriteDescriptorSet write_descriptor(m_set, index, 0, vk::DescriptorType::eUniformBuffer, {}, buffer_info);
        API->m_device.updateDescriptorSets(write_descriptor, {});
        return *this;
    }

    VulkanDescriptorSet& VulkanDescriptorSet::bind_sampler(VulkanSampler* sampler, BindingIndex index)
    {
        VulkanSampler*& current_sampler = m_sampler[index];
        if (current_sampler != sampler)
        {
            vk::DescriptorImageInfo image_info(sampler->m_sampler, {}, vk::ImageLayout::eShaderReadOnlyOptimal);
            vk::WriteDescriptorSet write_descriptor(m_set, index, 0, vk::DescriptorType::eSampler, image_info);
            API->m_device.updateDescriptorSets(write_descriptor, {});
            current_sampler = sampler;
        }
        return *this;
    }

    VulkanDescriptorSet& VulkanDescriptorSet::bind_texture(VulkanTexture* texture, BindingIndex index)
    {
        VulkanTexture*& current_texture = m_texture[index];

        if (current_texture != texture)
        {
            vk::DescriptorImageInfo image_info({}, texture->image_view(), vk::ImageLayout::eShaderReadOnlyOptimal);
            vk::WriteDescriptorSet write_descriptor(m_set, index, 0, vk::DescriptorType::eSampledImage, image_info);
            API->m_device.updateDescriptorSets(write_descriptor, {});
            current_texture = texture;
        }
        return *this;
    }

    VulkanDescriptorSet& VulkanDescriptorSet::bind_texture_combined(VulkanTexture* texture, VulkanSampler* sampler,
                                                                    BindingIndex index)
    {
        VulkanCombinedImageSampler& current_object = m_combined_image_sampler[index];

        if (current_object.texture != texture || current_object.sampler != sampler)
        {
            vk::DescriptorImageInfo image_info(sampler->m_sampler, texture->image_view(),
                                               vk::ImageLayout::eShaderReadOnlyOptimal);
            vk::WriteDescriptorSet write_descriptor(m_set, index, 0, vk::DescriptorType::eCombinedImageSampler, image_info);
            API->m_device.updateDescriptorSets(write_descriptor, {});
            current_object.texture = texture;
            current_object.sampler = sampler;
        }
        return *this;
    }
}// namespace Engine
