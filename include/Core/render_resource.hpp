#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/smart_ptr.hpp>
#include <Core/object.hpp>
#include <Core/structures.hpp>
#include <Core/task.hpp>

namespace Engine
{
	struct RHI_Object;
	struct RHI_BindingObject;
	struct RHI_Sampler;
	struct RHI_RenderTarget;
	struct RHI_Texture;
	struct RHI_Shader;
	struct RHI_Pipeline;
	struct RHI_Buffer;
	struct RHI_VertexBuffer;
	struct RHI_IndexBuffer;
	struct RHI_UniformBuffer;
	struct RHI_SSBO;
	struct RHI_RenderPass;

	class ENGINE_EXPORT RenderResourcePtrBase
	{
	protected:
		static void release(void* object);
	};

	template<typename T>
	class RenderResourcePtr final : private RenderResourcePtrBase
	{
	private:
		T* m_ptr = nullptr;

	public:
		RenderResourcePtr() = default;
		explicit RenderResourcePtr(T* ptr) : m_ptr(ptr) {}
		~RenderResourcePtr() { reset(); }

		RenderResourcePtr(const RenderResourcePtr&)            = delete;
		RenderResourcePtr& operator=(const RenderResourcePtr&) = delete;

		RenderResourcePtr(RenderResourcePtr&& other) noexcept
		{
			m_ptr       = other.m_ptr;
			other.m_ptr = nullptr;
		}

		RenderResourcePtr& operator=(RenderResourcePtr&& other) noexcept
		{
			if (this != &other)
			{
				reset();
				m_ptr       = other.m_ptr;
				other.m_ptr = nullptr;
			}
			return *this;
		}

		T* get() const { return m_ptr; }
		T* operator->() const { return m_ptr; }
		T& operator*() const { return *m_ptr; }
		operator T*() const { return m_ptr; }

		RenderResourcePtr& reset(T* new_ptr = nullptr)
		{
			if (m_ptr)
			{
				RenderResourcePtrBase::release(m_ptr);
			}
			m_ptr = new_ptr;
			return *this;
		}

		RenderResourcePtr& swap(RenderResourcePtr& other) noexcept
		{
			T* tmp      = m_ptr;
			m_ptr       = other.m_ptr;
			other.m_ptr = tmp;
			return *this;
		}

		RenderResourcePtr& operator=(T* ptr) { return reset(ptr); }
	};


	class ENGINE_EXPORT RenderResource : public Object
	{
		trinex_declare_class(RenderResource, Object);

	public:
		virtual RenderResource& init_render_resources();
		virtual RenderResource& release_render_resources();

		RenderResource& on_destroy() override;
		RenderResource& postload() override;
	};
}// namespace Engine
