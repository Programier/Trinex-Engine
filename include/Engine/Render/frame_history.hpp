#pragma once

namespace Engine
{
	class RHITexture;
	class RHIBuffer;

	struct ENGINE_EXPORT FrameHistory {
		static void release_texture(RHITexture* texture);
		static void release_buffer(RHIBuffer* buffer);

		template<typename T, void (*release)(T*)>
		class Resource
		{
		private:
			T* m_resource = nullptr;

		public:
			Resource() = default;

			Resource(T* ptr) : m_resource(ptr) {}
			~Resource() { reset(); }

			Resource(const Resource&)            = delete;
			Resource& operator=(const Resource&) = delete;

			Resource(Resource&& other) noexcept : m_resource(other.m_resource) { other.m_resource = nullptr; }

			Resource& operator=(Resource&& other) noexcept
			{
				if (this != &other)
				{
					reset();
					m_resource       = other.m_resource;
					other.m_resource = nullptr;
				}
				return *this;
			}

			void reset(T* ptr = nullptr)
			{
				if (m_resource)
					release(m_resource);
				m_resource = ptr;
			}

			T* get() const { return m_resource; }
			T* operator->() const { return m_resource; }
			T& operator*() const { return *m_resource; }
		};


		using Texture = Resource<RHITexture, release_texture>;
		using Buffer  = Resource<RHIBuffer, release_buffer>;

		Texture ssao  = nullptr;
		Texture depth = nullptr;
	};
}// namespace Engine
