#pragma once
#include "matrix.hpp"
#include <iostream>
#include <complex>

enum class axis { r, i, j, k };

constexpr axis axis_mul( axis a, axis b )
{
    return axis( int(a) ^ int(b) );
}

template< axis X >
struct basic
{
    double d;

    explicit operator double() const { return d; }

    basic operator +() const { return { +d }; }
    basic operator -() const { return { -d }; }

    basic operator +( basic a ) const { return { d + a.d }; }
    basic operator -( basic a ) const { return { d - a.d }; }
    basic operator *( double a ) const { return { d * a }; }
    basic operator /( double a ) const { return { d / a }; }

    friend basic operator *( double a, basic b ) { return { a * b.d }; }
    friend basic operator /( double a, basic b ) { return { a / -b.d }; } // wrong

    operator std::complex< double >() const requires (X <= axis::i)
    {
        std::complex< double > z;
        ((double*) &z)[ X ] = d;
        return z;
    }
};

basic< axis::i > operator "" i( long double d ) { return { double( d ) }; }
basic< axis::j > operator "" j( long double d ) { return { double( d ) }; }
basic< axis::k > operator "" k( long double d ) { return { double( d ) }; }

basic< axis::i > operator "" i( unsigned long long int n ) { return { double( n ) }; }
basic< axis::j > operator "" j( unsigned long long int n ) { return { double( n ) }; }
basic< axis::k > operator "" k( unsigned long long int n ) { return { double( n ) }; }

template< axis X >
std::ostream& operator <<( std::ostream& os, basic< X > n )
{
    if ( X == axis::r || n.d != 1 ) os << n.d;
    if ( X > axis::r ) os << "rijk"[ int( X ) ];
    return os;
}

template< axis X1, axis X2 >
basic< axis_mul( X1, X2 ) > operator *( basic< X1 > a, basic< X2 > b )
{
    if ( (X1 == axis::r || X2 == axis::r)
        || axis_mul( X1, X2 ) == axis::j ? X1 > X2 : X1 < X2 )
        return { a.d * b.d };
    return { -a.d * b.d };
}

template< axis X1, axis X2 >
basic< axis_mul( X1, X2 ) > operator /( basic< X1 > a, basic< X2 > b )
{
    if ( (X1 == axis::r || X2 == axis::r)
        || axis_mul( X1, X2 ) != axis::j ? X1 > X2 : X1 < X2 )
        return { a.d * b.d };
    return { -a.d * b.d };
}

struct quat
{
    double r = 0;
    union {
        vec3 v = { 0, 0, 0 };
        struct { double i, j, k; };
    };

    quat() = default;
    quat( const quat& ) = default;

    quat( double r, vec3 v = {} )
        : r { r }, v { v }
    {
    }

    quat( double r, double i, double j, double k )
        : quat( r, { i, j, k } )
    {
    }

    quat( std::complex< double > z, double j = 0, double k = 0 )
        : quat( z.real(), z.imag(), j, k )
    {
    }

    explicit quat( vec3 v ) : quat( 0, v )
    {
    }

    quat( axis x, double d = 1 ) : quat()
    {
        ((double*)this)[ int(x) ] = d;
    }

    template< axis X >
    quat( basic< X > b ) : quat( X, b.d )
    {
    }

    static quat angle_axis( double a, vec3 v )
    {
        return { cos( a / 2 ), v * sin( a / 2 ) };
    }

    quat operator +() const { return *this; }

    quat operator -() const { return { -r, -i, -j, -k }; }

    quat operator ~() const { return { r, -i, -j, -k }; }

    friend quat operator *( quat a, quat b )
    {
        return {
            a.r*b.r - a.i*b.i - a.j*b.j - a.k*b.k,
            a.r*b.i + a.i*b.r + a.j*b.k - a.k*b.j,
            a.r*b.j - a.i*b.k + a.j*b.r + a.k*b.i,
            a.r*b.k + a.i*b.j - a.j*b.i + a.k*b.r
        };
    }

    friend quat operator /( quat a, quat b )
    {
        double r2 = b.r*b.r + b.i*b.i + b.j*b.j + b.k*b.k;
        return {
            (a.r*b.r + a.i*b.i + a.j*b.j + a.k*b.k)/r2,
            (a.r*b.i - a.i*b.r - a.j*b.k + a.k*b.j)/r2,
            (a.r*b.j + a.i*b.k - a.j*b.r - a.k*b.i)/r2,
            (a.r*b.k - a.i*b.j + a.j*b.i - a.k*b.r)/r2
        };
    }

    friend quat operator +( quat a, quat b )
    {
        return { a.r+b.r, a.i+b.i, a.j+b.j, a.k+b.k };
    }

    friend quat operator -( quat a, quat b )
    {
        return { a.r-b.r, a.i-b.i, a.j-b.j, a.k-b.k };
    }

    friend std::ostream& operator <<( std::ostream& os, quat q )
    {
        auto func = [&]( double d ) {
            if ( d != 1 && d != -1 ) os << d;
            else if ( os.flags() & std::ios_base::showpos || d < 0 )
                os << (d < 0 ? '-' : '+');
        };
        auto f = os.flags();
        if ( q.r ) { os << q.r << std::showpos; }
        if ( q.i ) { func( q.i ); os << 'i' << std::showpos; }
        if ( q.j ) { func( q.j ); os << 'j' << std::showpos; }
        if ( q.k ) { func( q.k ); os << 'k'; }
        os.flags( f );
        return os;
    }

    vec3 rotate( vec3 v ) const
    {
        quat r = *this;
        return (r * quat(v) * ~r).v;
    }

    static const quat R;
    static const quat I;
    static const quat J;
    static const quat K;

    explicit operator matrix< 3 >() const
    {
        return matrix< 3 > {
            1 - 2*j*j - 2*k*k,  0 + 2*i*j - 2*k*r,  0 + 2*i*k + 2*j*r,
            0 + 2*i*j + 2*k*r,  1 - 2*i*i - 2*k*k,  0 + 2*j*k - 2*i*r,
            0 + 2*i*k - 2*j*r,  0 + 2*j*k + 2*i*r,  1 - 2*i*i - 2*j*j,
        };
    }
};

const quat quat::R { 1, 0, 0, 0 };
const quat quat::I { 0, 1, 0, 0 };
const quat quat::J { 0, 0, 1, 0 };
const quat quat::K { 0, 0, 0, 1 };

template< axis X1, axis X2 >
quat operator +( basic< X1 > a, basic< X2 > b ) { return quat(a) + quat(b); }

template< axis X1, axis X2 >
quat operator -( basic< X1 > a, basic< X2 > b ) { return quat(a) - quat(b); }

template< axis X >
quat operator +( basic< X > a, double b ) { return quat(a) + quat(b); }
template< axis X >
quat operator +( double a, basic< X > b ) { return quat(a) + quat(b); }

template< axis X >
quat operator -( basic< X > a, double b ) { return quat(a) - quat(b); }
template< axis X >
quat operator -( double a, basic< X > b ) { return quat(a) - quat(b); }

inline void test_quat()
{
    using r_t = basic< axis::r >;
    using i_t = basic< axis::i >;
    using j_t = basic< axis::j >;
    using k_t = basic< axis::k >;

    r_t a = 3i * 2i;
    j_t q = 3j + 2j;
    quat p = 3j + 2k;
    quat u = 3k + 2;

    quat j = axis::j;

    std::cout << a << '\n'
        << q << '\n'
        << p << '\n'
        << u << '\n'
        << 1k << '\n'
        << (1 + 1i - 1j + 2k) << '\n'
        << (q + p + u) * 3i + 1 << '\n';
}
