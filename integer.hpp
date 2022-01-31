#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <array>
#include <vector>
#include <type_traits>
#include <sstream>
#include <compare>

#include "console_color.hpp"

namespace {
using uint = unsigned int;
using ulong = unsigned long long;
}

std::string ltrim( const std::string& s, char c )
{
    auto lpos = s.find_first_not_of( c );
    if ( lpos == std::string::npos )
        lpos = s.length() - 1;
    return s.substr( lpos );
}

std::string rtrim( const std::string& s, char c )
{
    auto lpos = s.find_last_not_of( c );
    if ( lpos != std::string::npos )
        ++lpos;
    return s.substr( 0, lpos );
}

std::string lex_add( std::string_view a, std::string_view b )
{
    int alen = a.length();
    int blen = b.length();
    int alen2 = alen;
    int blen2 = blen;
    if ( size_t a0 = a.find_first_not_of( '0' ); a0 != std::string::npos ) alen2 -= a0;
    if ( size_t b0 = b.find_first_not_of( '0' ); b0 != std::string::npos ) blen2 -= b0;

    int len = std::max( alen2, blen2 );
    std::string sum( len + 1, '0' );

    for ( int i = 0; i <= len; ++i )
    {
        char x = i < alen ? a[ alen - i - 1 ] - '0' : 0;
        char y = i < blen ? b[ blen - i - 1 ] - '0' : 0;
        char c = sum[ len - i ] - '0' + x + y;

        while ( c > 9 )
        {
            c -= 10;
            sum[ len - i - 1 ]++;
        }
        sum[ len - i ] = c + '0';
    }
    return sum;
}

std::string lex_mul2( std::string_view s )
{
    uint slen = s.length();
    std::string d( slen + 1, '0' );
    char carry = 0;

    for ( int i = slen - 1; i >= 0; --i )
    {
        char x = 2 * (s[ i ] - '0') + carry;
        carry = 0;

        while ( x > 9 )
        {
            x -= 10;
            ++carry;
        }
        d[ i + 1 ] = x + '0';
    }
    d[ 0 ] = carry + '0';
    return ltrim( d, '0' );
}

std::string lex_div2( std::string_view s, bool& rem )
{
    if ( size_t start = s.find_first_not_of( '0' ); start != std::string::npos )
        s = s.substr( start );

    uint slen = s.length();
    bool will_shrink = !s.empty() && s[ 0 ] == '1';
    std::string d( slen - will_shrink, '\0' );

    char carry = 0;
    for ( uint i = 0; i < slen; ++i )
    {
        char x = s[ i ] - '0';
        x += carry * 10;
        carry = x & 1;
        x /= 2;
        if ( i > 0 || !will_shrink )
            d[ i - will_shrink ] = x + '0';
    }
    rem = carry != 0;
    return d;
}

template< uint Bits >
std::array< std::string, Bits > init_powers()
{
    std::array< std::string, Bits > powers { "1" };
    for ( uint n = 1; n < Bits; ++n )
        powers[ n ] = lex_mul2( powers[ n - 1 ] );
    return powers;
}

std::string_view* allocate_powers( uint max_bits )
{
    static uint bits = max_bits;
    static std::string_view* powers = new std::string_view[ bits ];
    return nullptr;
}

extern const std::string* const INTEGER_POWERS;

#define INIT_INTEGER_POWERS( MAX_BITS ) \
const auto integer_string_powers = init_powers< MAX_BITS >(); \
const std::string* const INTEGER_POWERS = integer_string_powers.data();

template< uint Bits = 32, typename IntType = signed >
struct integer
{
    static_assert( std::is_same_v< signed, IntType > || std::is_same_v< unsigned, IntType > );
    static constexpr bool IS_SIGNED = std::is_same_v< signed, IntType >;
    static constexpr bool IS_UNSIGNED = std::is_same_v< unsigned, IntType >;
    using primitive32_t = std::conditional_t< IS_SIGNED, int, uint >;
    using primitive64_t = std::conditional_t< IS_SIGNED, long long, ulong >;

    static constexpr uint NUM_BITS = Bits;
    static constexpr uint NUM_INTS = (Bits + 31) / 32;
    static constexpr uint INT_MASK = ~(Bits % 32 ? ~0 << Bits % 32 : 0);
    
    std::array< uint, NUM_INTS > ints;

    constexpr integer() noexcept : ints { 0 } {}
  
    integer( const integer& ) = default;
  
    constexpr bool is_negative() const
    {
        return IS_SIGNED && ((ints.back() >> (NUM_BITS - 1) % 32) & 1);
    }

    void extend_sign()
    {
        if constexpr ( NUM_BITS % 32 != 0 )
            if ( !is_negative() )
                ints.back() &= INT_MASK;
            else
                ints.back() |= ~INT_MASK;
    }

    integer( primitive32_t n ) : ints { (uint) n }
    {
        std::memset( ints.data() + 1, n < 0 ? ~0 : 0, (NUM_INTS - 1) * 4 );
        if constexpr ( NUM_BITS < 32 )
            extend_sign();
    }
  
    template< class = std::enable_if_t< (NUM_INTS > 1) > >
    integer( primitive64_t n ) : ints { (uint) n, uint( n >> 32 ) }
    {
        std::memset( ints.data() + 2, n < 0 ? ~0 : 0, (NUM_INTS - 2) * 4 );
        if constexpr ( NUM_BITS < 64 )
            extend_sign();
    }
  
    template< uint Bits0, typename IntT > requires (Bits0 < NUM_BITS)
    explicit( std::same_as< IntT, signed > && IS_UNSIGNED )
    integer( const integer< Bits0, IntT >& x ) : ints {}
    {
        const uint X_SIZE = x.ints.size();
        std::memcpy( ints.data(), x.ints.data(), X_SIZE * 4 );
        std::memset( ints.data() + X_SIZE, x < 0 ? ~0 : 0, (NUM_INTS - X_SIZE) * 4 );
    }
  
    template< uint Bits0, typename IntT >
    requires (Bits0 <= NUM_BITS)
    explicit operator integer< Bits0, IntT >() const
    {
        integer< Bits0, IntT > result;
        std::memcpy( &result.ints, &ints, (Bits0 + 7) / 8 );
        result.extend_sign();
        return result;
    }

private:
    explicit integer( std::array< ulong, NUM_INTS + 1 > carry ) : ints {}
    {
        for ( int i = 0; i < NUM_INTS; ++i )
        {
            ints[ i ] = (uint) carry[ i ];
            carry[ i + 1 ] += uint( carry[ i ] >> 32 );
        }
        extend_sign();
    }
public:
  
    integer operator ~() const
    {
        integer flipped;
        for ( int i = 0; i < NUM_INTS; ++i )
            flipped.ints[ i ] = ~ints[ i ];
        return flipped;
    }
  
    integer operator +() const
    {
        return *this;
    }
  
    friend integer operator +( integer x, integer y )
    {
        x.add( y );
        x.extend_sign();
        return x;
    }
  
    integer operator -() const
    {
        return operator ~() + 1;
    }
  
    friend integer operator -( integer x, integer y )
    {
        return x + -y;
    }
  
    friend integer operator *( integer x, integer y )
    {
        bool is_negative = x.is_negative() != y.is_negative();
        x = x.abs();
        y = y.abs();
    
        std::array< integer, NUM_INTS > products;
        for ( int i = 0; ulong a : x.ints )
        {
            std::array< ulong, NUM_INTS + 1 > carry { 0 };
            for ( int j = 0; ulong b : y.ints )
                carry[ j++ ] = a * b;

            products[ i++ ] = integer( carry );
        }
      
        integer sum;
        for ( size_t i = 0; i < NUM_INTS; ++i )
            sum.add( products[ i ], i );
        sum.extend_sign();
  
        return is_negative ? -sum : sum;
    }
  
    integer operator /( integer x ) const
    {
        return divmod( x )[ 0 ];
    }
  
    integer operator %( integer x ) const
    {
        return divmod( x )[ 1 ];
    }
  
    integer operator |( integer x ) const
    {
        for ( int i = 0; i < NUM_INTS; ++i )
            x.ints[ i ] |= ints[ i ];
        x.extend_sign();
        return x;
    }
  
    integer operator &( integer x ) const
    {
        for ( int i = 0; i < NUM_INTS; ++i )
            x.ints[ i ] &= ints[ i ];
        x.extend_sign();
        return x;
    }
  
    integer operator ^( integer x ) const
    {
        for ( int i = 0; i < NUM_INTS; ++i )
            x.ints[ i ] ^= ints[ i ];
        x.extend_sign();
        return x;
    }
  
    std::strong_ordering operator<=>( integer x ) const
    {
        for ( int i = NUM_INTS - 1; i >= 0; --i )
            if ( ints[ i ] > x.ints[ i ] )
                return std::strong_ordering::greater;
            else if ( ints[ i ] < x.ints[ i ] )
                return std::strong_ordering::less;
        return std::strong_ordering::equal;
    }

    friend integer operator <<( integer x, uint shl )
    {
        if ( shl >= NUM_BITS ) return {};
        x <<= shl;
        x.extend_sign();
        return x;
    }
  
    friend integer operator >>( integer x, uint shr )
    {
        if ( shr >= NUM_BITS ) return x.is_negative() ? -1 : 0;
        x >>= shr;
        x.extend_sign();
        return x;
    }
  
    integer& operator ++()
    {
        return *this += 1;
    }
  
    integer& operator --()
    {
        return *this -= 1;
    }
  
    integer operator ++(int)
    {
        auto tmp = *this;
        operator ++();
        return tmp;
    }
  
    integer operator --(int)
    {
        auto tmp = *this;
        operator --();
        return tmp;
    }

    integer& operator <<=( uint x )
    {
        int chunks = x / 32;
        int shift = x % 32;
    
        if ( chunks > 0 )
        {
            for ( int i = NUM_INTS - 1; i - chunks >= 0; --i )
                ints[ i ] = ints[ i - chunks ];
            for ( int i = chunks - 1; i >= 0; --i )
                ints[ i ] = 0;
        }
        if ( shift > 0 )
        {
            uint next_carry = 0;
            for ( int i = chunks; i < NUM_INTS; ++i )
            {
                uint& digit = ints[ i ];
                uint carry = next_carry;
                next_carry = digit >> (32 - shift);
                digit <<= shift;
                digit |= carry;
            }
        }
        return *this;
    }
  
    integer& operator >>=( uint x )
    {
        int chunks = x / 32;
        int shift = x % 32;
        bool neg = is_negative();
    
        if ( chunks > 0 )
        {
            for ( int i = 0; i + chunks < NUM_INTS; ++i )
                ints[ i ] = ints[ i + chunks ];
            for ( int i = chunks + 1; i < NUM_INTS; ++i )
                ints[ i ] = neg ? ~0 : 0;
        }
        if ( shift > 0 )
        {
            uint next_carry = neg ? ~0 << (32 - shift) : 0;
            for ( int i = NUM_INTS - 1 - chunks; i >= 0; --i )
            {
                uint& digit = ints[ i ];
                uint carry = next_carry;
                next_carry = digit << (32 - shift);
                digit >>= shift;
                digit |= carry;
            }
        }
        return *this;
    }

protected:
  
    void add( const integer& x, size_t n = 0 )
    {
        ulong carry = 0;
        for ( size_t i = n; i < NUM_INTS; ++i )
        {
            ulong a = ints[ i ] + carry + x.ints[ i - n ];
            ints[ i ] = (uint) a;
            carry = uint( a >> 32 );
        }
    }
  
public:

    integer abs() const
    {
        return is_negative() ? operator -() : *this;
    }
  
    std::array< integer, 2 > divmod( integer x ) const
    {
        integer div = 0;
        integer mod = abs();
        integer divisor = x.abs();

        while ( mod >= divisor )
        {
            mod -= divisor;
            ++div;
        }

        return is_negative() != x.is_negative()
            ? std::array { -div, mod }
            : std::array { div, mod };
    }
  
    explicit operator std::string() const
    {
        std::string num = "0";
        integer x = abs();
    
        for ( uint d = 0; d < NUM_INTS; ++d ) // digit
            for ( uint b = 0; b < 32 && d * 32 + b < NUM_BITS - IS_SIGNED; ++b ) // bit
                if ( (x.ints[ d ] >> b) & 1 )
                    num = lex_add( num, INTEGER_POWERS[ d * 32 + b ] );
    
        num = ltrim( num, '0' );
        return is_negative() ? "-" + num : num;
    }

    explicit integer( std::string_view str ) : ints {}
    {
        bool negative = str[ 0 ] == '-';
        std::string num( str.substr( negative || str[ 0 ] == '+' ) );
        
        for ( size_t bit = 0; num != "0" && bit < NUM_BITS - IS_SIGNED; ++bit )
        {
            bool odd; num = lex_div2( num, odd );
            ints[ bit / 32 ] |= uint( odd ) << (bit % 32);
        }
        if ( negative )
            ints = operator -().ints;
    }

    #define ASN_OP( OP ) \
    integer& operator OP##=( const integer& x ) { \
        return *this = *this OP x; \
    }
  
    ASN_OP( + )
    ASN_OP( - )
    ASN_OP( * )
    ASN_OP( / )
    ASN_OP( % )
    ASN_OP( ^ )
    ASN_OP( & )
    ASN_OP( | )
    
    #undef ASN_OP
};

template< typename IntType > struct integer< 0, IntType >;

template< uint Bits >
integer< Bits, signed > abs( const integer< Bits, signed >& x )
{
    return x.abs();
}

template< uint Bits, typename IntType >
auto bits( const integer< Bits, IntType >& x )
{
    return [x]( std::ostream& os ) -> std::ostream&
    {
        ConsoleColorFormat color( 0 );
        color.intensity = 1;
        os << color;

        for ( int j = 31; j >= 0; --j )
        {
            if ( j == (Bits - 1) % 32 )
                os << ConsoleColorFormat {};
            os << char( (x.ints.back() >> j & 1) + '0' );
        }

        for ( int i = x.ints.size() - 2; i >= 0; --i )
        {
            os << ' ';
            for ( int j = 31; j >= 0; --j )
                os << char( (x.ints[ i ] >> j & 1) + '0' );
        }
        return os;
    };
}

template< uint Bits, typename IntType >
std::ostream& operator <<( std::ostream& os, const integer< Bits, IntType >& x )
{
    return os << (std::string) x;
}

template< typename T >
decltype(auto) same( T&& a, T&& b ) { return std::forward< T >( a ); }

template< typename Func >
auto operator <<( std::ostream& os, Func&& func ) -> decltype( same( std::invoke( func, os ), os ) )
{
    return func( os );
}
