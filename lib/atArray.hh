#pragma once

#include <cstdint>
#include <string.h>
#include <string_view>
#include <type_traits>

#pragma pack(push, 1)

template <typename T> struct atArrayIterator
{
    T      &arr;
    int32_t idx;

    explicit atArrayIterator (T &arr, int32_t idx = 0) : arr (arr), idx (idx) {}

    atArrayIterator &
    operator++ ()
    {
        idx++;
        return *this;
    }

    auto &
    operator* ()
    {
        return arr[idx];
    }

    bool
    operator!= (const atArrayIterator &other) const
    {
        return this->idx != other.idx;
    }
};

template <typename T> struct atArrayBase
{
    T Data;

    using ElemType      = std::decay_t<decltype (Data[0])>;
    using Iterator      = atArrayIterator<T>;
    using ConstIterator = atArrayIterator<const T>;

    ElemType &
    operator[] (size_t ix)
    {
        return Data[ix];
    }

    const ElemType &
    operator[] (size_t ix) const
    {
        return Data[ix];
    }

    constexpr Iterator
    begin ()
    {
        return Iterator (Data);
    }

    constexpr ConstIterator
    begin () const
    {
        return ConstIterator (Data);
    }
};

template <typename T> struct atArrayGetSizeWrapperObject
{
    T *Data;

    T &
    operator[] (size_t ix)
    {
        return *reinterpret_cast<T *> (reinterpret_cast<char *> (Data)
                                       + T::GetSize () * ix);
    }

    const T &
    operator[] (size_t ix) const
    {
        return *reinterpret_cast<T *> (reinterpret_cast<char *> (Data)
                                       + T::GetSize () * ix);
    }
};

template <typename T = void *>
struct atArrayGetSizeWrapper
    : public atArrayBase<atArrayGetSizeWrapperObject<T>>
{
    using Base = atArrayBase<atArrayGetSizeWrapperObject<T>>;
    using Iterator = typename Base::Iterator;
    using ConstIterator = typename Base::ConstIterator;

    uint16_t Size;
    uint16_t Capacity;

    atArrayGetSizeWrapper () = default;

    atArrayGetSizeWrapper (uint8_t *data, uint16_t size)
    {
        this->Data     = *reinterpret_cast<decltype (this->Data) *> (&data);
        this->Size     = size;
        this->Capacity = size;
    }

    constexpr Iterator
    end ()
    {
        return Iterator (Base::Data, Size);
    }

    constexpr ConstIterator
    end () const
    {
        return ConstIterator (Base::Data, Size);
    }
};

template <typename T = void *> struct atArray : public atArrayBase<T *>
{
    using Base = atArrayBase<T *>;
    using Iterator = typename Base::Iterator;
    using ConstIterator = typename Base::ConstIterator;
    
    uint16_t Size;
    uint16_t Capacity;

    constexpr Iterator
    end ()
    {
        return Iterator (Base::Data, Size);
    }

    constexpr ConstIterator
    end () const
    {
        return ConstIterator (Base::Data, Size);
    }
};

template <typename T, uint32_t Size>
struct atFixedArray : public atArrayBase<T[Size]>
{
    using Base          = atArrayBase<T[Size]>;
    using Iterator      = typename Base::Iterator;
    using ConstIterator = typename Base::ConstIterator;

    constexpr Iterator
    end ()
    {
        return Iterator(Base::Data, Size);
    }

    constexpr ConstIterator
    end () const
    {
        return ConstIterator(Base::Data, Size);
    }
};

#pragma pack(pop)
