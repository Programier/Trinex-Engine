#pragma once

#include <Core/engine_types.hpp>


namespace Engine
{
    template<typename T, typename EnumType>
    concept FlagsConcept = !std::is_enum_v<T> && std::is_integral_v<T>;


    enum class FlagsOperator
    {
        And,
        Or,
        Xor,
    };

    template<typename FlagsType = BitMask>
    struct Flags {
        BitMask flags;

        FORCE_INLINE Flags(FlagsType flags) : flags(static_cast<BitMask>(flags))
        {}

        template<typename T = BitMask>
            requires FlagsConcept<T, FlagsType>
        FORCE_INLINE Flags(T mask = 0) : flags(static_cast<BitMask>(mask))
        {}

        template<typename... Args>
        FORCE_INLINE Flags(FlagsOperator op, FlagsType first, Args... args) : flags(BitMask(first))
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

        FORCE_INLINE Flags(const Flags&)            = default;
        FORCE_INLINE Flags& operator=(const Flags&) = default;

        FORCE_INLINE operator BitMask() const
        {
            return flags;
        }

        FORCE_INLINE operator BitMask&()
        {
            return flags;
        }

        FORCE_INLINE operator const BitMask&() const
        {
            return flags;
        }

        FORCE_INLINE operator bool() const
        {
            return flags != 0;
        }

        FORCE_INLINE bool has_all(Flags mask) const
        {
            return (flags & mask.flags) == mask.flags;
        }

        FORCE_INLINE bool has_any(Flags mask) const
        {
            return (flags & mask.flags) != 0;
        }

        FORCE_INLINE Flags& set(Flags mask)
        {
            flags |= mask.flags;
            return *this;
        }

        FORCE_INLINE Flags& remove(Flags mask)
        {
            flags &= ~mask.flags;
            return *this;
        }

        FORCE_INLINE Flags& toggle(Flags mask)
        {
            flags ^= mask.flags;
            return *this;
        }

        FORCE_INLINE Flags& clear_all()
        {
            flags = 0;
            return *this;
        }

        FORCE_INLINE size_t count_set_bits() const
        {
            size_t count = 0;
            BitMask mask = 1;
            while (mask)
            {
                if (flags & mask)
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
            return flags & mask.flags;
        }

        FORCE_INLINE Flags operator|(Flags mask) const
        {
            return flags | mask.flags;
        }

        FORCE_INLINE Flags operator^(Flags mask) const
        {
            return flags ^ mask.flags;
        }

        FORCE_INLINE Flags operator~() const
        {
            return ~flags;
        }

        FORCE_INLINE Flags& operator&=(Flags mask)
        {
            flags &= mask.flags;
            return *this;
        }

        FORCE_INLINE Flags& operator|=(Flags mask)
        {
            flags |= mask.flags;
            return *this;
        }

        FORCE_INLINE Flags& operator^=(Flags mask)
        {
            flags ^= mask.flags;
            return *this;
        }
    };
}// namespace Engine
