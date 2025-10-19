#include <Core/etl/allocator.hpp>
#include <Core/etl/vector.hpp>
#include <RHI/context.hpp>
#include <cstring>

namespace Engine
{
	template<typename T>
	class RHIStateStack
	{
	private:
		static T s_default;

		T* m_data         = nullptr;
		size_t m_size     = 0;
		size_t m_capacity = 0;

		void grow()
		{
			size_t new_capacity = m_capacity ? m_capacity * 2 : 4;
			T* new_data         = Allocator<T>::allocate(new_capacity);

			if (m_data)
			{
				std::memcpy(new_data, m_data, sizeof(T) * m_capacity);
			}

			m_data     = new_data;
			m_capacity = new_capacity;
		}

	public:
		RHIStateStack() : m_data(Allocator<T>::allocate(4)) {}

		~RHIStateStack()
		{
			if (m_data)
			{
				Allocator<T>::deallocate(m_data);
			}
		}

		const T& push(const T& value)
		{
			if (m_size == m_capacity)
				grow();

			return m_data[m_size++] = value;
		}

		const T& pop()
		{
			--m_size;
			if (m_size == 0)
				return s_default;
			return m_data[m_size - 1];
		}

		inline void reset() { m_size = 0; }
	};

	template<typename T>
	T RHIStateStack<T>::s_default = T();

	template<>
	RHIColorComponent RHIStateStack<RHIColorComponent>::s_default = RHIColorComponent::RGBA;

	struct RHIContext::State {
		RHIStateStack<RHIPrimitiveTopology> topology;
		RHIStateStack<RHIPolygonMode> polygon_mode;
		RHIStateStack<RHICullMode> cull_mode;
		RHIStateStack<RHIFrontFace> front_face;
		RHIStateStack<RHIColorComponent> write_mask;

		inline void reset()
		{
			topology.reset();
			polygon_mode.reset();
			cull_mode.reset();
			front_face.reset();
			write_mask.reset();
		}
	};

	RHIContext::RHIContext() : m_state(trx_new State()) {}

	RHIContext::~RHIContext()
	{
		trx_delete m_state;
	}

	RHIContext& RHIContext::push_primitive_topology(RHIPrimitiveTopology topology)
	{
		return primitive_topology(m_state->topology.push(topology));
	}

	RHIContext& RHIContext::push_polygon_mode(RHIPolygonMode mode)
	{
		return polygon_mode(m_state->polygon_mode.push(mode));
	}

	RHIContext& RHIContext::push_cull_mode(RHICullMode mode)
	{
		return cull_mode(m_state->cull_mode.push(mode));
	}

	RHIContext& RHIContext::push_front_face(RHIFrontFace face)
	{
		return front_face(m_state->front_face.push(face));
	}

	RHIContext& RHIContext::push_write_mask(RHIColorComponent mask)
	{
		return write_mask(m_state->write_mask.push(mask));
	}

	RHIContext& RHIContext::pop_primitive_topology()
	{
		return primitive_topology(m_state->topology.pop());
	}

	RHIContext& RHIContext::pop_polygon_mode()
	{
		return polygon_mode(m_state->polygon_mode.pop());
	}

	RHIContext& RHIContext::pop_cull_mode()
	{
		return cull_mode(m_state->cull_mode.pop());
	}

	RHIContext& RHIContext::pop_front_face()
	{
		return front_face(m_state->front_face.pop());
	}

	RHIContext& RHIContext::pop_write_mask()
	{
		return write_mask(m_state->write_mask.pop());
	}

	RHIContext& RHIContext::reset_state()
	{
		m_state->reset();
		return *this;
	}
}// namespace Engine
