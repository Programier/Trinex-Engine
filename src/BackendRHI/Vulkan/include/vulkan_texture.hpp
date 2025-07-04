#pragma once
#include <Core/etl/set.hpp>
#include <Core/etl/vector.hpp>
#include <RHI/rhi.hpp>
#include <vk_mem_alloc.h>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	class VulkanTexture : public VulkanDeferredDestroy<RHI_Texture>
	{
	public:
		struct ViewDesc {
			vk::Format format           = {};
			vk::ImageViewType view_type = {};
			uint16_t first_mip          = 0;
			uint16_t first_array_slice  = 0;
			uint16_t mip_levels         = ~0;
			uint16_t array_size         = ~0;
			
			template<typename T>
			static ViewDesc from_generic(const T* view, const VulkanTexture* texture);
			
			static ViewDesc from(const RHITextureDescSRV* view, const VulkanTexture* texture);
			static ViewDesc from(const RHITextureDescUAV* view, const VulkanTexture* texture);
			static ViewDesc from(const RHITextureDescRTV* view, const VulkanTexture* texture);
			static ViewDesc from(const RHITextureDescDSV* view, const VulkanTexture* texture);

			FORCE_INLINE bool operator==(const ViewDesc& other) const noexcept
			{
				return format == other.format && view_type == other.view_type && first_mip == other.first_mip &&
				       first_array_slice == other.first_array_slice && mip_levels == other.mip_levels &&
				       array_size == other.array_size;
			}

			FORCE_INLINE bool operator!=(const ViewDesc& other) const noexcept { return !(*this == other); }
		};

		template<typename Value>
		struct View {
			View* next  = nullptr;
			Value value = nullptr;
			ViewDesc desc;
		};

		template<typename Value>
		struct HeadView {
			View<Value>* next = nullptr;
			Value value       = nullptr;
		};

	private:
		HeadView<struct VulkanTextureSRV*> m_srv;
		HeadView<struct VulkanTextureUAV*> m_uav;
		HeadView<struct VulkanTextureRTV*> m_rtv;
		HeadView<struct VulkanTextureDSV*> m_dsv;

		VmaAllocation m_allocation = VK_NULL_HANDLE;
		RHITextureCreateFlags m_flags;

		vk::Image m_image;
		vk::ImageLayout m_layout;
		vk::ImageAspectFlags m_aspect;
		vk::Format m_format;
		vk::Extent3D m_extent;

		uint16_t m_layers_count;
		uint16_t m_mips_count;

		VulkanTexture& create_views();

	public:
		virtual vk::ImageViewType image_view_type() const = 0;

		inline vk::Image image() const { return m_image; }
		inline vk::ImageLayout layout() const { return m_layout; }
		inline vk::ImageAspectFlags aspect() const { return m_aspect; }
		inline vk::Format format() const { return m_format; }
		inline vk::Extent3D extent() const { return m_extent; }
		inline uint16_t layer_count() const { return m_layers_count; }
		inline uint16_t mipmap_count() const { return m_mips_count; }

		inline vk::ImageType image_type() const
		{
			switch (image_view_type())
			{
				case vk::ImageViewType::e2D:
				case vk::ImageViewType::e2DArray: return vk::ImageType::e2D;

				case vk::ImageViewType::e3D:
				case vk::ImageViewType::eCube:
				case vk::ImageViewType::eCubeArray: return vk::ImageType::e3D;

				default: return vk::ImageType::e1D;
			}
		}

		VulkanTexture& create(RHIColorFormat color_format, Vector3u size, uint32_t mips, RHITextureCreateFlags flags);
		VulkanTexture& update(const RHITextureUpdateDesc& desc);
		void change_layout(vk::ImageLayout new_layout);

		RHI_ShaderResourceView* as_srv(const RHITextureDescSRV* desc) override;
		RHI_UnorderedAccessView* as_uav(const RHITextureDescUAV* desc) override;
		RHI_RenderTargetView* as_rtv(const RHITextureDescRTV* desc) override;
		RHI_DepthStencilView* as_dsv(const RHITextureDescDSV* desc) override;

		byte* map(RHIMappingAccess access, const RHIMappingRange* range = nullptr) override;
		void unmap(const RHIMappingRange* range = nullptr) override;
		~VulkanTexture();
	};

	template<vk::ImageViewType m_type>
	class VulkanTypedTexture : public VulkanTexture
	{
	public:
		using VulkanTexture::VulkanTexture;
		vk::ImageViewType image_view_type() const override { return m_type; }
	};
}// namespace Engine
