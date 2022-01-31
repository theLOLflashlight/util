#pragma once
#include <cmath>
#include <iostream>

#define stack_alloc( TYPE, COUNT ) \
    ((TYPE*) alloca( sizeof( TYPE ) * (COUNT) ))

template< typename T >
struct tvec3
{
    T x, y, z;
    
    static const tvec3 X, Y, Z;
    static const tvec3 zero, one, NaN;
    
    tvec3 operator -() const {
        return { -x, -y, -z };
    }
    tvec3 operator *( T s ) const {
        return { x * s, y * s, z * s };
    }
    tvec3 operator /( T s ) const {
        return operator *( 1 / s );
        //return { x / s, y / s, z / s };
    }
    tvec3& operator *=( T s ) {
        return *this = *this * s;
    }
    tvec3& operator /=( T s ) {
        return *this = *this / s;
    }
    
    friend tvec3 operator +( tvec3 a, tvec3 b ) {
        return { a.x + b.x, a.y + b.y, a.z + b.z };
    }
    friend tvec3 operator -( tvec3 a, tvec3 b ) {
        return { a.x - b.x, a.y - b.y, a.z - b.z };
    }
    friend tvec3 operator *( tvec3 a, tvec3 b ) {
        return { a.x * b.x, a.y * b.y, a.z * b.z };
    }
    friend tvec3 operator /( tvec3 a, tvec3 b ) {
        return { a.x / b.x, a.y / b.y, a.z / b.z };
    }
    
    tvec3& operator +=( tvec3 v ) {
        return *this = *this + v;
    }
    tvec3& operator -=( tvec3 v ) {
        return *this = *this - v;
    }
    tvec3& operator *=( tvec3 v ) {
        return *this = *this * v;
    }
    tvec3& operator /=( tvec3 v ) {
        return *this = *this / v;
    }
    
    bool operator ==( tvec3 v ) const {
        return x == v.x && y == v.y && z == v.z;
    }
    
    double length() const { using namespace std;
        return sqrt( x*x + y*y + z*z );
    }
    tvec3 normalized() const {
        return *this / length();
    }
    void normalize() {
        operator /=( length() );
    }
};

using vec3 = tvec3< double >;

namespace std
{
    template< typename T >
    struct hash< tvec3< T > >
    {
        size_t operator ()( const tvec3< T >& v ) const
        {
            std::hash< T > hasher;
            auto x = hasher( v.x ) & 0xFFFFFFFF;
            auto y = hasher( v.y ) << 32;
            auto z = hasher( v.z );
            return (x | y) ^ z;
        }
    };
}

const vec3 vec3::X { 1, 0, 0 };
const vec3 vec3::Y { 0, 1, 0 };
const vec3 vec3::Z { 0, 0, 1 };
const vec3 vec3::zero { 0, 0, 0 };
const vec3 vec3::one { 1, 1, 1 };
const vec3 vec3::NaN { NAN, NAN, NAN };

vec3 cross( vec3 a, vec3 b )
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

double dot( vec3 a, vec3 b )
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

double length( vec3 v )
{
    return sqrt( dot( v, v ) );
}

vec3 normalize( vec3 v )
{
    return v / length( v );
}

vec3 lerp( vec3 a, vec3 b, double t )
{
    return a*(1 - t) + b*t;
}

vec3 bezier( double t, vec3 a, vec3 b, vec3 c )
{
    return lerp( lerp( a, b, t ), lerp( b, c, t ), t );
}

vec3 bezier( double t, const vec3 verts[], size_t size )
{
    switch ( vec3* verts_; size ) {
    default : // [4..2^64)
        verts_ = stack_alloc( vec3, size );
        while ( size > 3 )
        {
            for ( int i = 0; i < size - 1; ++i )
                verts_[i] = lerp( verts[i], verts[i+1], t );
            verts = verts_;
            --size;
        }
    case 3 : return bezier( t, verts[0], verts[1], verts[2] );
    case 2 : return lerp( verts[0], verts[1], t );
    case 1 : return verts[0];
    case 0 : return vec3::NaN;
    }
}

vec3 bezier( double t, std::initializer_list< vec3 > il )
{
    return bezier( t, il.begin(), il.size() );
}
