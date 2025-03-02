#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	enum class FlagsOperator
	{
		And,
		Or,
		Xor,
	};


	template<typename FlagsType = BitMask, typename ValueType = BitMask>
	struct Flags final {
	public:
		using Type  = FlagsType;
		using Value = ValueType;

	private:
		ValueType m_flags;

	public:
		FORCE_INLINE Flags() : m_flags(0)
		{}

		FORCE_INLINE Flags(BitMask flags) : m_flags(flags)
		{}

		FORCE_INLINE Flags(FlagsType flags) : m_flags(static_cast<BitMask>(flags))
		{}

		template<typename... Args>
		FORCE_INLINE Flags(FlagsOperator op, FlagsType first, Args... args) : m_flags(BitMask(first))
		{
			switch (op)
			{
				case FlagsOperator::And:
					(((*this) &= args), ...);
					break;
				case FlagsOperator::Or:
					(((*this) |= args), ...);
					break;
				case FlagsOperator::Xor:
					(((*this) ^= args), ...);
					break;
			}
		}

		FORCE_INLINE Flags(const Flags& new_flags) : Flags(static_cast<BitMask>(new_flags.m_flags))
		{}

		FORCE_INLINE Flags& operator=(const Flags& new_flags)
		{
			if (this != &new_flags)
			{
				m_flags = static_cast<BitMask>(new_flags.m_flags);
			}
			return *this;
		}

		FORCE_INLINE operator BitMask() const
		{
			return m_flags;
		}

		FORCE_INLINE operator FlagsType() const
		{
			return static_cast<FlagsType>(m_flags);
		}

		FORCE_INLINE bool has_all(Flags mask) const
		{
			return (m_flags & mask.m_flags) == mask.m_flags;
		}

		FORCE_INLINE bool has_any(Flags mask) const
		{
			return (m_flags & mask.m_flags) != 0;
		}

		FORCE_INLINE Flags& set(Flags mask)
		{
			m_flags |= mask.m_flags;
			return *this;
		}

		FORCE_INLINE Flags& remove(Flags mask)
		{
			m_flags &= ~mask.m_flags;
			return *this;
		}

		FORCE_INLINE Flags& toggle(Flags mask)
		{
			m_flags ^= mask.m_flags;
			return *this;
		}

		FORCE_INLINE Flags& clear_all()
		{
			m_flags = 0;
			return *this;
		}

		FORCE_INLINE size_t count_set_bits() const
		{
			size_t count = 0;
			BitMask mask = 1;
			while (mask)
			{
				if (m_flags & mask)
					count++;
				mask <<= 1;
			}
			return count;
		}

		FORCE_INLINE bool operator()(Flags mask) const
		{
			return has_all(mask);
		}

		FORCE_INLINE Flags& operator()(Flags mask, bool flag)
		{
			if (flag)
			{
				return set(mask);
			}
			return remove(mask);
		}

		FORCE_INLINE Flags operator&(Flags mask) const
		{
			return m_flags & mask.m_flags;
		}

		FORCE_INLINE Flags operator|(Flags mask) const
		{
			return m_flags | mask.m_flags;
		}

		FORCE_INLINE Flags operator^(Flags mask) const
		{
			return m_flags ^ mask.m_flags;
		}

		FORCE_INLINE Flags operator~() const
		{
			return ~m_flags;
		}

		FORCE_INLINE Flags& operator&=(Flags mask)
		{
			m_flags &= mask.m_flags;
			return *this;
		}

		FORCE_INLINE Flags& operator|=(Flags mask)
		{
			m_flags |= mask.m_flags;
			return *this;
		}

		FORCE_INLINE Flags& operator^=(Flags mask)
		{
			m_flags ^= mask.m_flags;
			return *this;
		}
	};

}// namespace Engine
