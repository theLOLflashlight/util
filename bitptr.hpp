#pragma once
#include <compare>

using byte = signed char;
using ubyte = unsigned char;
using ushort = unsigned short;
using uint = unsigned int;
using ulong = unsigned long;

struct bitref
{
    ubyte* const ptr;
    size_t const bit : 3;

    constexpr operator bool() const
    {
        return *ptr & (1 << bit);
    }

    constexpr const bitref& operator =( bool b ) const
    {
        *ptr = (*ptr & ~(1 << bit)) | (b << bit);
        return *this;
    }

    friend constexpr void swap( bitref a, bitref b )
    {
        bool x = a, y = b;
        a = y; b = x;
    }

    constexpr struct bitptr operator &() const;
};


struct bitptr
{
    size_t   bit : 3  = 0;
    intptr_t ptr : 61 = 0;

    bitptr() = default;
    bitptr( const bitptr& ) = default;
    constexpr bitptr( nullptr_t ) : bitptr() {}

    constexpr bitptr( void* ptr, size_t off )
        : bit { off % 8 }
        , ptr { (intptr_t) ptr + intptr_t( off / 8 ) }
    {
    }

    constexpr bitref operator *() const
    {
        return { (ubyte*) ptr, bit };
    }

    constexpr bitref operator []( size_t pos ) const
    {
        return *(*this + pos);
    }

    constexpr bitptr& operator ++()
    {
        ++as_int();
        return *this;
    }

    constexpr bitptr& operator --()
    {
        --as_int();
        return *this;
    }

    constexpr bitptr operator ++(int)
    {
        auto tmp = *this;
        operator ++();
        return tmp;
    }

    constexpr bitptr operator --(int)
    {
        auto tmp = *this;
        operator --();
        return tmp;
    }

    friend constexpr bitptr operator +( bitptr p, size_t n )
    {
        p.as_int() += n;
        return p;
    }

    friend constexpr bitptr operator -( bitptr p, size_t n )
    {
        p.as_int() -= n;
        return p;
    }

    friend constexpr intptr_t operator -( bitptr x, bitptr y )
    {
        return x.as_int() - y.as_int();
    }

    friend constexpr bool operator ==( bitptr x, bitptr y )
    {
        return x.as_int() == y.as_int();
    }

    friend constexpr std::strong_ordering operator<=>( bitptr x, bitptr y )
    {
        return x.as_int() <=> y.as_int();
    }

private:

    constexpr intptr_t& as_int()
    {
        return *(intptr_t*) this;
    }

    constexpr const intptr_t& as_int() const
    {
        return *(const intptr_t*) this;
    }

public:

    constexpr explicit operator intptr_t&() { return as_int(); }
    constexpr explicit operator const intptr_t&() const { return as_int(); }
};

static_assert( sizeof( bitptr ) == sizeof( bool* ) );

constexpr bitptr bitref::operator &() const
{
    return { ptr, bit };
}
