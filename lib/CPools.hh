#pragma once

#include <cstdint>
#include <iterator>

template <typename T> class CPool
{
public:
    T *      m_pObjects;
    uint8_t *m_pMask;
    uint32_t m_nMaxElements;
    uint32_t m_nElementSize;
    uint32_t m_nFirstFree;
    uint32_t m_nUnk;
    uint32_t m_nCount;
    uint32_t m_nUnk2;

    /* Returns number of elements in the pool */
    uint32_t
    GetCount ()
    {
        return m_nCount & 0x3fffffff;
    }

    /* Returns number of free slots in the pool */
    uint32_t
    GetFree ()
    {
        return m_nMaxElements - GetCount ();
    }

    // Because why not, right? :P
    class CPoolIterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = T *;
        using difference_type   = void;
        using pointer           = T **;
        using reference         = T *;

        CPool<T> *pool  = nullptr;
        int32_t   index = 0;

        explicit CPoolIterator (CPool<T> *pool, int32_t index = 0)
        {
            this->pool  = pool;
            this->index = index;
        }

        CPoolIterator &
        operator++ ()
        {
            for (index++; index < pool->m_nMaxElements; index++)
                if (pool->IsValidIndex (index))
                    return *this;

            index = pool->m_nMaxElements;
            return *this;
        }

        CPoolIterator
        operator++ (int)
        {
            CPoolIterator retval = *this;
            ++(*this);
            return retval;
        }

        bool
        operator== (CPoolIterator other) const
        {
            return index == other.index;
        }
        bool
        operator!= (CPoolIterator other) const
        {
            return !(*this == other);
        }

        CPoolIterator &
        operator-- ()
        {
            for (index--; index > -1; index--)
                if (pool->IsValidIndex (index))
                    return *this;

            index = pool->m_nMaxElements;
            return *this;
        }

        CPoolIterator
        operator-- (int)
        {
            CPoolIterator retval = *this;
            --(*this);
            return retval;
        }

        reference
        operator* ()
        {
            return pool->GetAt (index);
        }
    };

    CPoolIterator
    begin ()
    {
        return ++CPoolIterator (this, -1);
    }

    CPoolIterator
    end ()
    {
        return CPoolIterator (this, this->m_nMaxElements);
    }

    /*******************************************************/
    bool
    IsValidIndex (int index) const
    {
        return index < m_nMaxElements && (m_pMask[index] & 128) == 0;
    }

    /*******************************************************/
    T *
    GetAt (int index)
    {
        if (!IsValidIndex (index))
            return nullptr;

        return reinterpret_cast<T *> (reinterpret_cast<uint8_t *> (m_pObjects)
                                      + m_nElementSize * index);
    }

    /*******************************************************/
    T *
    GetAtHandle (int handle)
    {
        return GetAt (handle >> 8);
    }
};

static_assert (sizeof (CPool<void *>) == 40);

struct CVehicleStruct;
struct CVehicleStreamRequestGfx;

class CPools
{
public:
    inline static CPool<CVehicleStruct> **g_pVehicleStructPool;
    inline static CPool<CVehicleStreamRequestGfx> *
        *g_pVehicleStreamRequestGfxPool;

    inline static CPool<CVehicleStruct> *&
    GetVehicleStructPool ()
    {
        return *g_pVehicleStructPool;
    };

    inline static auto *&
    GetVehicleStreamRequestGxtPool ()
    {
        return *g_pVehicleStreamRequestGfxPool;
    };

    static void InitialisePatterns ();
};
