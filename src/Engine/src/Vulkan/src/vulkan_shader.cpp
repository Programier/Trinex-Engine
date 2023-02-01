#include <vulkan_api.hpp>
#include <vulkan_shader.hpp>

#define in :

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

    VulkanShader& VulkanShader::init(const ShaderParams& params)
    {
        _M_shader_params = params;

        std::vector<vk::ShaderModule*> shader_modules;
        create_shader_module(_M_shader_params.binaries.vertex, shader_modules);
        create_shader_module(_M_shader_params.binaries.fragment, shader_modules);
        create_shader_module(_M_shader_params.binaries.compute, shader_modules);
        create_shader_module(_M_shader_params.binaries.geometry, shader_modules);

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

        VkPipelineLayoutCreateInfo pipeline_layout_info = {};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 0;
        pipeline_layout_info.pushConstantRangeCount = 0;

        _M_pipeline_layout = API->_M_device.createPipelineLayout(pipeline_layout_info);

        std::vector<vk::DynamicState> dynamic_states = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

        vk::PipelineDynamicStateCreateInfo dynamic_info({}, dynamic_states);

        vk::GraphicsPipelineCreateInfo pipeline_info({}, pipeline_shader_stage_create_infos, &vertex_input_info,
                                                     &input_assembly, nullptr, &viewport_state, &rasterizer,
                                                     &multisampling, nullptr, &color_blending, &dynamic_info,
                                                     _M_pipeline_layout, API->_M_render_pass, 0, {});


        auto pipeline_result = API->_M_device.createGraphicsPipeline({}, pipeline_info);

        if (pipeline_result.result != vk::Result::eSuccess)
        {
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

        return *this;
    }

    VulkanShader::~VulkanShader()
    {
        API->_M_device.destroyPipeline(_M_pipeline);
        API->_M_device.destroyPipelineLayout(_M_pipeline_layout);
    }

}// namespace Engine
