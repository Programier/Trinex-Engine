#include <iostream>
#include <thread>
#include <vulkan_api.hpp>
#include <vulkan_shader.hpp>


#define SHADER_DATA _M_shader_params.binaries

namespace Engine
{

    static void create_shader_module(const FileBuffer& buffer, std::vector<vk::ShaderModule*>& modules)
    {
        if (buffer.empty())
        {
            modules.push_back(nullptr);
            return;
        }

        vk::ShaderModuleCreateInfo create_info(vk::ShaderModuleCreateFlags(), buffer.size(),
                                               reinterpret_cast<const uint32_t*>(buffer.data()));
        modules.push_back(new vk::ShaderModule(API->_M_device.createShaderModule(create_info)));
    }


    static vk::ShaderStageFlagBits get_stage_by_index(std::size_t index)
    {
        static const vk::ShaderStageFlagBits flags[] = {
                vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment, vk::ShaderStageFlagBits::eCompute,
                vk::ShaderStageFlagBits::eGeometry};

        if (index > 3)
            throw std::runtime_error("Undefined shader stage index");
        return flags[index];
    }

    VulkanShader::VulkanShader()
    {}

    void* VulkanShader::get_instance_data()
    {
        return this;
    }

    VulkanShader& VulkanShader::allocate_uniform_buffers()
    {
        _M_ubo_buffer.resize(MAIN_FRAMEBUFFERS_COUNT);

        for (Counter i = 0; i < MAIN_FRAMEBUFFERS_COUNT; i++)
        {
            Counter var_index = 0;
            _M_ubo_buffer[i]._M_buffers.resize(_M_shader_params.uniform_variables.size());

            for (auto& variable : _M_shader_params.uniform_variables)
            {
                auto& ubo = _M_ubo_buffer[i]._M_buffers[var_index++];

                _M_ubo_buffer[i]._M_buffer_map[variable.name] = &ubo;

                vk::DeviceSize buffer_size = variable.size;
                API->create_buffer(buffer_size, vk::BufferUsageFlagBits::eUniformBuffer,
                                   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                                   ubo._M_buffer, ubo._M_device_memory);
                ubo._M_memory = API->_M_device.mapMemory(ubo._M_device_memory, 0, buffer_size);
                ubo.size = variable.size;
            }
        }

        return *this;
    }

    VulkanShader& VulkanShader::create_descriptor_layout()
    {

        std::vector<vk::DescriptorSetLayoutBinding> ubo_layout_bindings(_M_shader_params.uniform_variables.size());

        ArrayIndex index = 0;

        for (auto& variable : _M_shader_params.uniform_variables)
        {
            ubo_layout_bindings[index++] = vk::DescriptorSetLayoutBinding(
                    variable.binding, vk::DescriptorType::eUniformBuffer, 1,
                    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, nullptr);
        }

        vk::DescriptorSetLayoutCreateInfo layout_info({}, ubo_layout_bindings);
        _M_descriptor_set_layout = API->_M_device.createDescriptorSetLayout(layout_info);
        return *this;
    }

    VulkanShader& VulkanShader::init(const ShaderParams& params)
    {
        _M_shader_params = params;

        std::sort(_M_shader_params.uniform_variables.begin(), _M_shader_params.uniform_variables.end(),
                  [](const ShaderUniformVariable& a, const ShaderUniformVariable& b) -> bool {
                      return a.binding < b.binding;
                  });

        for (auto& ell : _M_shader_params.uniform_variables)
        {
            std::clog << ell.binding << std::endl;
        }

        create_descriptor_layout();

        std::vector<vk::ShaderModule*> shader_modules;
        create_shader_module(SHADER_DATA.vertex, shader_modules);
        create_shader_module(SHADER_DATA.fragment, shader_modules);
        create_shader_module(SHADER_DATA.compute, shader_modules);
        create_shader_module(SHADER_DATA.geometry, shader_modules);

        std::vector<vk::PipelineShaderStageCreateInfo> pipeline_shader_stage_create_infos;

        std::size_t index = 0;
        for (auto module : shader_modules)
        {
            if (module)
            {
                pipeline_shader_stage_create_infos.emplace_back(vk::PipelineShaderStageCreateFlags(),
                                                                get_stage_by_index(index), *module,
                                                                _M_shader_params.name.c_str());
            }
            ++index;
        }

        if (pipeline_shader_stage_create_infos.empty())
        {
            throw std::runtime_error("Failed to create Shader");
        }


        vk::PipelineVertexInputStateCreateInfo vertex_input_info(vk::PipelineVertexInputStateCreateFlags(), 0, nullptr,
                                                                 0, nullptr);

        vk::PipelineInputAssemblyStateCreateInfo input_assembly(vk::PipelineInputAssemblyStateCreateFlags(),
                                                                vk::PrimitiveTopology::eTriangleList, VK_FALSE);


        vk::Viewport viewport(0.f, 0.f, static_cast<float>(API->_M_swap_chain->_M_extent.width),
                              static_cast<float>(API->_M_swap_chain->_M_extent.height), API->_M_min_depth,
                              API->_M_max_depth);

        vk::Rect2D scissor({0, 0}, API->_M_swap_chain->_M_extent);

        vk::PipelineViewportStateCreateInfo viewport_state({}, 1, &viewport, 1, &scissor);

        vk::PipelineRasterizationStateCreateInfo rasterizer({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill,
                                                            vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise,
                                                            VK_FALSE, {}, {}, {}, 1.f);

        vk::PipelineMultisampleStateCreateInfo multisampling({}, vk::SampleCountFlagBits::e1, VK_FALSE);

        vk::PipelineColorBlendAttachmentState color_blend_attachment(VK_FALSE);

        color_blend_attachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                                 vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

        static std::array<float, 4> blend_constants = {0.f, 0.f, 0.f, 0.f};

        vk::PipelineColorBlendStateCreateInfo color_blending({}, VK_FALSE, vk::LogicOp::eCopy, 1,
                                                             &color_blend_attachment, blend_constants);

        vk::PipelineLayoutCreateInfo pipeline_layout_info({}, _M_descriptor_set_layout);

        _M_pipeline_layout = API->_M_device.createPipelineLayout(pipeline_layout_info);

        std::vector<vk::DynamicState> dynamic_states = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        vk::PipelineDynamicStateCreateInfo dynamic_state_info({}, dynamic_states);

        vk::GraphicsPipelineCreateInfo pipeline_info({}, pipeline_shader_stage_create_infos, &vertex_input_info,
                                                     &input_assembly, nullptr, &viewport_state, &rasterizer,
                                                     &multisampling, nullptr, &color_blending, &dynamic_state_info,
                                                     _M_pipeline_layout, API->_M_render_pass, 0, {});

        auto pipeline_result = API->_M_device.createGraphicsPipeline({}, pipeline_info);

        if (pipeline_result.result != vk::Result::eSuccess)
        {
            destroy_descriptor_layout();
            API->_M_device.destroyPipeline(_M_pipeline);
            throw std::runtime_error("Failed to create pipeline");
        }

        _M_pipeline = pipeline_result.value;

        for (auto module : shader_modules)
        {
            if (module)
            {
                API->_M_device.destroyShaderModule(*module);
                delete module;
            }
        }

        allocate_uniform_buffers();

        vk::DescriptorPoolSize pool_size(vk::DescriptorType::eUniformBuffer,
                                         MAIN_FRAMEBUFFERS_COUNT * _M_shader_params.uniform_variables.size());
        vk::DescriptorPoolCreateInfo pool_info({}, MAIN_FRAMEBUFFERS_COUNT, pool_size);
        _M_descriptor_pool = API->_M_device.createDescriptorPool(pool_info);
        create_descriptor_sets();

        _M_inited = true;
        return *this;
    }

    VulkanShader& VulkanShader::use()
    {
        API->_M_current_command_buffer->bindPipeline(vk::PipelineBindPoint::eGraphics, _M_pipeline);
        API->_M_current_command_buffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _M_pipeline_layout, 0, 1,
                                                           &_M_ubo_buffer[API->_M_current_frame]._M_descriptor_set, 0,
                                                           nullptr);
        return *this;
    }

    VulkanShader& VulkanShader::set_value(const String& name, void* data)
    {
        auto it = _M_ubo_buffer[API->_M_current_frame]._M_buffer_map.find(name);
        if (it == _M_ubo_buffer[API->_M_current_frame]._M_buffer_map.end())
            return *this;

        const auto& buffer = *(*it).second;
        std::memcpy(buffer._M_memory, data, buffer.size);
        return *this;
    }

    VulkanShader& VulkanShader::destroy_descriptor_layout()
    {
        API->_M_device.destroyDescriptorSetLayout(_M_descriptor_set_layout);
        return *this;
    }

    VulkanShader& VulkanShader::destroy_uniform_buffers()
    {
        for (auto& buffers : _M_ubo_buffer)
        {
            for (auto& buffer : buffers._M_buffers)
            {
                API->_M_device.destroyBuffer(buffer._M_buffer);
                API->_M_device.freeMemory(buffer._M_device_memory);
            }
        }

        _M_ubo_buffer.clear();

        return *this;
    }

    VulkanShader& VulkanShader::create_descriptor_sets()
    {

        std::vector<vk::DescriptorSetLayout> layouts(MAIN_FRAMEBUFFERS_COUNT, _M_descriptor_set_layout);
        vk::DescriptorSetAllocateInfo alloc_info(_M_descriptor_pool, layouts);

        auto sets = API->_M_device.allocateDescriptorSets(alloc_info);

        for (Counter i = 0; i < MAIN_FRAMEBUFFERS_COUNT; i++)
        {
            auto& ubo = _M_ubo_buffer[i];
            ubo._M_descriptor_set = sets[i];

            std::vector<vk::DescriptorBufferInfo> _M_buffer_infos(ubo._M_buffers.size());

            Counter index = 0;
            for (auto& buffer : ubo._M_buffers)
            {
                _M_buffer_infos[index++] = vk::DescriptorBufferInfo(buffer._M_buffer, 0, buffer.size);
            }

            vk::WriteDescriptorSet write_descriptor(ubo._M_descriptor_set, 0, 0, vk::DescriptorType::eUniformBuffer, {},
                                                    _M_buffer_infos);

            API->_M_device.updateDescriptorSets(1, &write_descriptor, 0, nullptr);
        }


        return *this;
    }

    VulkanShader& VulkanShader::destroy_descriptor_sets()
    {

        return *this;
    }

    VulkanShader::~VulkanShader()
    {
        if (_M_inited)
        {
            destroy_descriptor_sets();
            API->_M_device.destroyDescriptorPool(_M_descriptor_pool);
            destroy_uniform_buffers();
            destroy_descriptor_layout();
            API->_M_device.destroyPipeline(_M_pipeline);
            API->_M_device.destroyPipelineLayout(_M_pipeline_layout);
        }
    }

}// namespace Engine
