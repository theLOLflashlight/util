#pragma once
#include <sstream>
#include <compare>
#include <array>

namespace si {

constexpr int abs( int value )
{
    uint32_t temp = value >> 31;     // make a mask of the sign bit
    value ^= temp;                   // toggle the bits if value is negative
    value += temp & 1;               // add one if value was negative
    return value;
}

struct magnitude
{
    static constexpr const char* symbols[]{
        "kg", "m", "s", "A", "K", "mol", "cd",
        "Hz", "N", "Pa", "J", "W", "C", "V", "F", "ohm",
        "S", "Wb", "T", "H", "lx", "Sv", "kat",
    };

    static constexpr size_t num_symbols = sizeof( symbols ) / sizeof( symbols[0] );
    static constexpr size_t num_base = 7;
    static constexpr size_t num_derived = num_symbols - num_base;

    int kg, m, s, A, K, mol, cd;

    struct output_t : std::array< int, num_symbols >
    {
        using base = std::array< int, num_symbols >;
        //int Hz, N, Pa, J, W, C, V, F, ohm, S, Wb, T, H, lx, Sv, kat;

        output_t() = default;
        output_t( const output_t& ) = default;
        constexpr output_t( const magnitude& mag )
            : base { mag.kg, mag.m, mag.s, mag.A, mag.K, mag.mol, mag.cd }
        {
        }

        constexpr int length() const
        {
            int len = 0;
            for ( int i : *this )
                len += abs( i );
            return len;
        }

        constexpr output_t& operator +=( const output_t& x )
        {
            for ( int i = 0; i < num_symbols; ++i )
                (*this)[ i ] += x[ i ];
            return *this;
        }

        constexpr output_t& operator -=( const output_t& x )
        {
            for ( int i = 0; i < num_symbols; ++i )
                (*this)[ i ] -= x[ i ];
            return *this;
        }
    };

    static constexpr output_t try_mul( output_t o, size_t i );
    static constexpr output_t try_div( output_t o, size_t i );

    constexpr output_t output_symbols() const
    {
        output_t output = *this, best;
        do {
            best = output;
            for ( int i = 0; i < num_derived; ++i )
            {
                output_t mul = try_mul( output, i );
                output_t div = try_div( output, i );
                bool small_mul = mul.length() < div.length();
                output_t better = small_mul ? mul : div;
                if ( better.length() < best.length() )
                {
                    do mul = (small_mul ? try_mul : try_div)( better, i );
                    while ( mul.length() < better.length() && ((better = mul), true) );
                    best = better;
                }
            }
        } while ( output.length() > best.length() && ((output = best), true) );

        return output;
    }

    constexpr magnitude& operator +=( magnitude x )
    {
        kg += x.kg; m += x.m; s += x.s; A += x.A; K += x.K; mol += x.mol; cd += x.cd;
        return *this;
    }

    constexpr magnitude& operator -=( magnitude x )
    {
        kg -= x.kg; m -= x.m; s -= x.s; A -= x.A; K -= x.K; mol -= x.mol; cd -= x.cd;
        return *this;
    }

    constexpr magnitude& operator *=( magnitude x )
    {
        kg *= x.kg; m *= x.m; s *= x.s; A *= x.A; K *= x.K; mol *= x.mol; cd *= x.cd;
        return *this;
    }

    constexpr magnitude& operator /=( magnitude x )
    {
        kg /= x.kg; m /= x.m; s /= x.s; A /= x.A; K /= x.K; mol /= x.mol; cd /= x.cd;
        return *this;
    }

    friend constexpr magnitude operator +( magnitude a, magnitude b ) { return a += b; }
    friend constexpr magnitude operator -( magnitude a, magnitude b ) { return a -= b; }
    friend constexpr magnitude operator *( magnitude a, magnitude b ) { return a *= b; }
    friend constexpr magnitude operator /( magnitude a, magnitude b ) { return a /= b; }

    friend std::ostream& operator <<( std::ostream& os, output_t input )
    {
        std::ostringstream bottom;
        for ( int i = 0; i < num_symbols; ++i )
        {
            if ( int amt = input[ i ] )
            {
                auto& oss = (amt > 0 ? os : bottom);
                oss << symbols[ i ];
                if ( amt = abs( amt ); amt != 1 )
                    oss << '^' << amt;
            }
        }

        if ( std::string text = bottom.str(); !text.empty() )
            os << '/' << text;
        return os;
    }
};

namespace base
{
    constexpr auto kilogram = magnitude( 1, 0, 0, 0, 0, 0, 0 );
    constexpr auto meter = magnitude( 0, 1, 0, 0, 0, 0, 0 );
    constexpr auto second = magnitude( 0, 0, 1, 0, 0, 0, 0 );
    constexpr auto ampere = magnitude( 0, 0, 0, 1, 0, 0, 0 );
    constexpr auto kelvin = magnitude( 0, 0, 0, 0, 1, 0, 0 );
    constexpr auto mol = magnitude( 0, 0, 0, 0, 0, 1, 0 );
    constexpr auto candela = magnitude( 0, 0, 0, 0, 0, 0, 1 );

    constexpr magnitude units[] = {
        kilogram, meter, second, ampere, kelvin, mol, candela
    };
    constexpr size_t num_units = sizeof( units ) / sizeof( magnitude );

    static_assert( num_units == magnitude::num_base );
}

namespace derived
{
    constexpr auto hertz = magnitude( 0, 0, -1, 0, 0, 0, 0 );
    constexpr auto newton = magnitude( 1, 1, -2, 0, 0, 0, 0 );
    constexpr auto pascal = magnitude( 1, -1, -2, 0, 0, 0, 0 );
    constexpr auto joule = magnitude( 1, 2, -2, 0, 0, 0, 0 );
    constexpr auto watt = magnitude( 1, 2, -3, 0, 0, 0, 0 );
    constexpr auto coulomb = magnitude( 0, 0, 1, 1, 0, 0, 0 );
    constexpr auto volt = magnitude( 1, 2, -3, -1, 0, 0, 0 );
    constexpr auto farad = magnitude( -1, -2, 4, 2, 0, 0, 0 );
    constexpr auto ohm = magnitude( 1, 2, -3, -2, 0, 0, 0 );
    constexpr auto siemens = magnitude( -1, -2, 3, 2, 0, 0, 0 );
    constexpr auto weber = magnitude( 1, 2, -2, -1, 0, 0, 0 );
    constexpr auto tesla = magnitude( 1, 0, -2, -1, 0, 0, 0 );
    constexpr auto henry = magnitude( 1, 2, -2, -2, 0, 0, 0 );
    constexpr auto lux = magnitude( 0, -2, 0, 0, 0, 0, 1 );
    constexpr auto sievert = magnitude( 0, 2, -2, 0, 0, 0, 1 );
    constexpr auto katal = magnitude( 0, 0, -1, 0, 0, 1, 1 );

    constexpr magnitude units[] = {
        hertz, newton, pascal, joule, watt, coulomb, volt, farad,
        ohm, siemens, weber, tesla, henry, lux, sievert, katal,
    };
    constexpr size_t num_units = sizeof( units ) / sizeof( magnitude );

    static_assert( num_units == magnitude::num_derived );
}

constexpr magnitude::output_t magnitude::try_mul( output_t o, size_t i )
{
    o -= derived::units[ i ];
    o[ base::num_units + i ]++;
    return o;
}

constexpr magnitude::output_t magnitude::try_div( output_t o, size_t i )
{
    o += derived::units[ i ];
    o[ base::num_units + i ]--;
    return o;
}

template< typename T, magnitude M >
struct unit
{
    T value;

    constexpr unit operator +() const { return { +value }; }
    constexpr unit operator -() const { return { -value }; }
    friend constexpr unit operator +( unit a, T b ) { return { a.value + b }; }
    friend constexpr unit operator +( T a, unit b ) { return { a + b.value }; }
    friend constexpr unit operator -( unit a, T b ) { return { a.value - b }; }
    friend constexpr unit operator -( T a, unit b ) { return { a - b.value }; }
    friend constexpr unit operator *( unit a, T b ) { return { a.value * b }; }
    friend constexpr unit operator *( T a, unit b ) { return { a * b.value }; }
    friend constexpr unit operator /( unit a, T b ) { return { a.value / b }; }
    friend constexpr unit operator /( T a, unit b ) { return { a / b.value }; }

    friend constexpr unit operator +( unit a, unit b ) { return { a.value + b.value }; }
    friend constexpr unit operator -( unit a, unit b ) { return { a.value - b.value }; }

    template< int N >
    constexpr auto pow() const
    {
        unit< T, M * magnitude { N, N, N, N, N, N, N } > power { 1 };
        for ( int i = 0; i < abs( N ); ++i )
            if constexpr ( N < 0 )
                power.value /= value;
            else power.value *= value;
        return power;
    }

    template< typename U >
    requires std::convertible_to< T, U >
    explicit( !std::same_as< U, std::common_type_t< T, U > > )
    constexpr operator unit< U, M >() const
    {
        return { U( value ) };
    }

    constexpr auto operator <=>( unit u ) const { return value <=> u.value; }

    explicit operator std::string() const
    {
        std::ostringstream oss;
        oss << (std::string) value;
        if constexpr ( M.s != 0 ) oss << " s^" << M.s;
        if constexpr ( M.m != 0 ) oss << " m^" << M.m;
        if constexpr ( M.kg != 0 ) oss << " kg^" << M.kg;
        if constexpr ( M.A != 0 ) oss << " A^" << M.A;
        if constexpr ( M.K != 0 ) oss << " K^" << M.K;
        if constexpr ( M.mol != 0 ) oss << " mol^" << M.mol;
        if constexpr ( M.cd != 0 ) oss << " cd^" << M.cd;
        return oss.str();
    }

    friend std::ostream& operator <<( std::ostream& os, unit u )
    {
        constexpr auto symbols = M.output_symbols();
        return os << u.value << symbols;
    }
};

template< typename T, magnitude M, typename U, magnitude N >
constexpr auto operator *( unit< T, M > a, unit< U, N > b )
    -> unit< std::common_type_t< T, U >, operator+( M, N ) >
{
    return { a.value * b.value };
}

template< typename T, magnitude M, typename U, magnitude N >
constexpr auto operator /( unit< T, M > a, unit< U, N > b )
    -> unit< std::common_type_t< T, U >,  operator-( M, N ) >
{
    return { a.value / b.value };
}

template< typename T >
constexpr unit< T, magnitude( 0, 0, 0, 0, 0, 0, 0 ) > unity { 1 };

template< typename T > constexpr unit< T, base::kilogram > kilogram { 1 };
template< typename T > constexpr unit< T, base::meter > meter { 1 };
template< typename T > constexpr unit< T, base::second > second { 1 };
template< typename T > constexpr unit< T, base::ampere > ampere { 1 };
template< typename T > constexpr unit< T, base::kelvin > kelvin { 1 };
template< typename T > constexpr unit< T, base::mol > mol { 1 };
template< typename T > constexpr unit< T, base::candela > candela { 1 };

template< typename T > constexpr unit< T, derived::hertz > hertz { 1 };
template< typename T > constexpr unit< T, derived::newton > newton { 1 };
template< typename T > constexpr unit< T, derived::pascal > pascal { 1 };
template< typename T > constexpr unit< T, derived::joule > joule { 1 };
template< typename T > constexpr unit< T, derived::watt > watt { 1 };
template< typename T > constexpr unit< T, derived::coulomb > coulomb { 1 };
template< typename T > constexpr unit< T, derived::volt > volt { 1 };
template< typename T > constexpr unit< T, derived::farad > farad { 1 };
template< typename T > constexpr unit< T, derived::ohm > ohm { 1 };
template< typename T > constexpr unit< T, derived::siemens > siemen { 1 };
template< typename T > constexpr unit< T, derived::weber > weber { 1 };
template< typename T > constexpr unit< T, derived::tesla > tesla { 1 };
template< typename T > constexpr unit< T, derived::henry > henry { 1 };
template< typename T > constexpr unit< T, derived::lux > lux { 1 };
template< typename T > constexpr unit< T, derived::sievert > sievert { 1 };
template< typename T > constexpr unit< T, derived::katal > katal { 1 };

namespace literals
{
    using int_lit = unsigned long long int;
    using float_lit = long double;

    constexpr auto operator "" kg( float_lit n ) { return n * kilogram< double >; }
    constexpr auto operator "" kg( int_lit n ) { return n * kilogram< long long >; }
    constexpr auto operator "" m( float_lit n ) { return n * meter< double >; }
    constexpr auto operator "" m( int_lit n ) { return n * meter< long long >; }
    constexpr auto operator "" s( float_lit n ) { return n * second< double >; }
    constexpr auto operator "" s( int_lit n ) { return n * second< long long >; }
    constexpr auto operator "" A( float_lit n ) { return n * ampere< double >; }
    constexpr auto operator "" A( int_lit n ) { return n * ampere< long long >; }
    constexpr auto operator "" K( float_lit n ) { return n * kelvin< double >; }
    constexpr auto operator "" K( int_lit n ) { return n * kelvin< long long >; }
    constexpr auto operator "" mol( float_lit n ) { return n * mol< double >; }
    constexpr auto operator "" mol( int_lit n ) { return n * mol< long long >; }
    constexpr auto operator "" cd( float_lit n ) { return n * candela< double >; }
    constexpr auto operator "" cd( int_lit n ) { return n * candela< long long >; }
}

} // namespace si

void test_unit( std::ostream& cout )
{
    using std::endl;
    using namespace si::literals;

    si::magnitude mag { 1, 2, -3, -1, 0, 0, 0 };

    auto mynum = 2.0kg * 3m;
    auto mynum2 = 2kg * 3.0m;
    auto mynum3 = 2.0kg + 3;
    auto mynum4 = (2.0kg * 3m).pow< 2 >();


    cout << mynum << endl;
    cout << (mynum / (2s * 2s)).pow< 2 >() << endl;
}
