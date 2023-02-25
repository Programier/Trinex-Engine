#pragma once

#include <Core/engine_types.hpp>
#include <unordered_map>
#include <vulkan_object.hpp>

namespace Engine
{
    struct VulkanShader : VulkanObject {

        struct UBO {
            vk::Buffer _M_buffer;
            vk::DeviceMemory _M_device_memory;
            void* _M_memory;
            size_t size;
        };

        struct UBO_Buffers{
            std::vector<UBO> _M_buffers;
            std::unordered_map<String, UBO*> _M_buffer_map;
            vk::DescriptorSet _M_descriptor_set;
        };

        ShaderParams _M_shader_params;
        vk::PipelineLayout _M_pipeline_layout;
        bool _M_has_descriptors = false;

        vk::DescriptorSetLayout* _M_descriptor_set_layout = nullptr;
        std::vector<UBO_Buffers> _M_ubo_buffer;

        vk::DescriptorPool _M_descriptor_pool;

        vk::Pipeline _M_pipeline;
        bool _M_inited = false;

        VulkanShader();
        void* get_instance_data() override;

    private:
        VulkanShader& destroy_descriptor_layout();
        VulkanShader& create_descriptor_layout();
        VulkanShader& allocate_uniform_buffers();
        VulkanShader& destroy_uniform_buffers();
        VulkanShader& create_descriptor_sets();
        std::vector<vk::VertexInputBindingDescription> get_binding_description();
        std::vector<vk::VertexInputAttributeDescription> get_attribute_description();
        std::vector<vk::DescriptorPoolSize> create_pool_size();

    public:
        VulkanShader& init(const ShaderParams& params);
        VulkanShader& use();
        VulkanShader& set_value(const String& name, void* data);
        VulkanShader& bind_texture(class VulkanTexture* texture, uint_t binding);
        ~VulkanShader();
    };
}// namespace Engine
