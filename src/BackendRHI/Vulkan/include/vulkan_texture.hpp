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
			union
			{
				struct {
					uint16_t first_array_slice     = 0;
					uint16_t array_size            = ~0;
					uint8_t first_mip              = 0;
					uint8_t mip_levels             = ~0;
					RHITextureType::Enum view_type = RHITextureType::Undefined;
				};
				uint64_t id;
			};


			FORCE_INLINE void normalize(VulkanTexture* texture) {}

			static ViewDesc from(const RHITextureDescSRV& view, const VulkanTexture* texture);
			static ViewDesc from(const RHITextureDescUAV& view, const VulkanTexture* texture);
			static ViewDesc from(const RHITextureDescRTV& view, const VulkanTexture* texture);
			static ViewDesc from(const RHITextureDescDSV& view, const VulkanTexture* texture);

			FORCE_INLINE bool operator==(const ViewDesc& other) const noexcept { return id == other.id; }
			FORCE_INLINE bool operator!=(const ViewDesc& other) const noexcept { return id != other.id; }
		};

		static_assert(sizeof(ViewDesc) == sizeof(uint64_t));

		template<typename Value>
		struct View {
			Value* value = nullptr;
			ViewDesc desc;
		};

	private:
		Vector<View<class VulkanTextureSRV>> m_srv;
		Vector<View<class VulkanTextureUAV>> m_uav;
		Vector<View<class VulkanTextureRTV>> m_rtv;
		Vector<View<class VulkanTextureDSV>> m_dsv;

		VmaAllocation m_allocation = VK_NULL_HANDLE;
		RHITextureCreateFlags m_flags;

		vk::Image m_image;
		vk::ImageLayout m_layout;
		vk::ImageAspectFlags m_aspect;
		vk::Format m_format;
		vk::Extent3D m_extent;

		uint16_t m_layers_count;
		uint8_t m_mips_count;

	public:
		virtual vk::ImageViewType image_view_type() const = 0;
		virtual RHITextureType texture_type() const       = 0;

		inline vk::Image image() const { return m_image; }
		inline vk::ImageLayout layout() const { return m_layout; }
		inline vk::ImageAspectFlags aspect() const { return m_aspect; }
		inline vk::Format format() const { return m_format; }
		inline vk::Extent3D extent() const { return m_extent; }
		inline uint16_t layer_count() const { return m_layers_count; }
		inline uint8_t mipmap_count() const { return m_mips_count; }

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

		RHI_ShaderResourceView* as_srv(RHITextureDescSRV desc) override;
		RHI_UnorderedAccessView* as_uav(RHITextureDescUAV desc) override;
		RHI_RenderTargetView* as_rtv(RHITextureDescRTV desc) override;
		RHI_DepthStencilView* as_dsv(RHITextureDescDSV desc) override;

		byte* map(RHIMappingAccess access, const RHIMappingRange* range = nullptr) override;
		void unmap(const RHIMappingRange* range = nullptr) override;
		~VulkanTexture();
	};

	template<vk::ImageViewType m_view_type, RHITextureType m_texture_type>
	class VulkanTypedTexture : public VulkanTexture
	{
	public:
		using VulkanTexture::VulkanTexture;
		vk::ImageViewType image_view_type() const override { return m_view_type; }
		RHITextureType texture_type() const override { return m_texture_type; }
	};
}// namespace Engine
