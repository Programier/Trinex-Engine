#pragma once
#include <Core/etl/set.hpp>
#include <Core/etl/vector.hpp>
#include <RHI/rhi.hpp>
#include <vk_mem_alloc.h>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>

namespace Trinex
{
	class VulkanContext;

	class VulkanTexture : public VulkanDeferredDestroy<RHITexture>
	{
	public:
		struct ViewDesc {
			union
			{
				struct {
					u16 first_array_slice;
					u16 array_size;
					u8 first_mip;
					u8 mip_levels;
					RHITextureType::Enum view_type;
				};
				u64 id;
			};


			ViewDesc() : first_array_slice(0), array_size(~0), first_mip(0), mip_levels(~0), view_type(RHITextureType::Undefined)
			{}

			FORCE_INLINE void normalize(VulkanTexture* texture) {}

			template<typename T>
			static ViewDesc from_base(const T* view, const VulkanTexture* texture);

			static ViewDesc from(const RHITextureDescSRV* view, const VulkanTexture* texture);
			static ViewDesc from(const RHITextureDescUAV* view, const VulkanTexture* texture);
			static ViewDesc from(const RHITextureDescRTV* view, const VulkanTexture* texture);
			static ViewDesc from(const RHITextureDescDSV* view, const VulkanTexture* texture);

			FORCE_INLINE bool operator==(const ViewDesc& other) const noexcept { return id == other.id; }
			FORCE_INLINE bool operator!=(const ViewDesc& other) const noexcept { return id != other.id; }
		};

		static_assert(sizeof(ViewDesc) == sizeof(u64));

		template<typename Value>
		struct View {
			Value* value = nullptr;
			ViewDesc desc;
		};

	protected:
		Vector<View<class VulkanTextureSRV>> m_srv;
		Vector<View<class VulkanTextureUAV>> m_uav;
		Vector<View<class VulkanTextureRTV>> m_rtv;
		Vector<View<class VulkanTextureDSV>> m_dsv;

		VmaAllocation m_allocation = VK_NULL_HANDLE;
		RHITextureFlags m_flags;

		vk::Image m_image;
		RHIAccess m_access;
		vk::Format m_format;
		vk::Extent3D m_extent;

		u16 m_layers_count;
		u8 m_mips_count;

	public:
		virtual vk::ImageViewType image_view_type() const = 0;
		virtual RHITextureType texture_type() const       = 0;

		inline vk::Image image() const { return m_image; }
		inline RHIAccess access() const { return m_access; }
		inline vk::ImageAspectFlags aspect() const
		{
			if (m_format == vk::Format::eD16Unorm || m_format == vk::Format::eD32Sfloat)
				return vk::ImageAspectFlagBits::eDepth;

			if (m_format == vk::Format::eD24UnormS8Uint)
				return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;

			return vk::ImageAspectFlagBits::eColor;
		}
		inline vk::Format format() const { return m_format; }
		inline vk::Extent3D extent() const { return m_extent; }
		inline u16 layer_count() const { return m_layers_count; }
		inline u8 mipmap_count() const { return m_mips_count; }
		inline bool is_cube_compatible() const
		{
			vk::ImageViewType type = image_view_type();
			return type == vk::ImageViewType::eCube || type == vk::ImageViewType::eCubeArray;
		}

		inline vk::ImageType image_type() const
		{
			switch (image_view_type())
			{
				case vk::ImageViewType::e2D:
				case vk::ImageViewType::e2DArray:
				case vk::ImageViewType::eCube:
				case vk::ImageViewType::eCubeArray: return vk::ImageType::e2D;
				case vk::ImageViewType::e3D: return vk::ImageType::e3D;

				default: return vk::ImageType::e1D;
			}
		}

		VulkanTexture& create(RHIColorFormat format, Vector3u size, u32 layers, u32 mips, RHITextureFlags flags);
		VulkanTexture& create(vk::Format format, Vector3u size, u32 layers, u32 mips, RHITextureFlags flags);
		VulkanTexture& barrier(VulkanContext* ctx, RHIAccess access);

		Vector3u size() const override;
		RHIShaderResourceView* as_srv(RHITextureDescSRV* desc) override;
		RHIUnorderedAccessView* as_uav(RHITextureDescUAV* desc) override;
		RHIRenderTargetView* as_rtv(RHITextureDescRTV* desc) override;
		RHIDepthStencilView* as_dsv(RHITextureDescDSV* desc) override;
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
}// namespace Trinex
