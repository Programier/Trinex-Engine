#pragma once

#include <Core/engine_types.hpp>


namespace Engine
{
    struct ENGINE_EXPORT Flags {
        BitMask flags;

        FORCE_INLINE Flags(BitMask mask = 0) : flags(mask)
        {}

        FORCE_INLINE Flags(const Flags&)            = default;
        FORCE_INLINE Flags& operator=(const Flags&) = default;

        FORCE_INLINE Flags& operator=(const BitMask& mask)
        {
            flags = mask;
            return *this;
        }

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


        FORCE_INLINE bool has_all(BitMask mask) const
        {
            return (flags & mask) == mask;
        }

        FORCE_INLINE bool has_any(BitMask mask) const
        {
            return (flags & mask) != 0;
        }

        FORCE_INLINE Flags& set(BitMask mask)
        {
            flags |= mask;
            return *this;
        }

        FORCE_INLINE Flags& remove(BitMask mask)
        {
            flags &= ~mask;
            return *this;
        }

        FORCE_INLINE Flags& toggle(BitMask mask)
        {
            flags ^= mask;
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

        FORCE_INLINE bool operator()(BitMask mask) const
        {
            return has_all(mask);
        }

        FORCE_INLINE Flags& operator()(BitMask mask, bool flag)
        {
            if (flag)
            {
                return set(mask);
            }
            return remove(flags);
        }
    };
}// namespace Engine
