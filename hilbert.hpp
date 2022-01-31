#pragma once
#include <iostream>
#include <array>
#include <type_traits>

// 3D integer vector
struct ivec3
{
    int x = 0;
    int y = 0;
    int z = 0;
};

template< size_t N > requires (N > 0)
struct uvec3
{
    using uint_t = std::conditional_t<
        N * 3 <= 8, unsigned char, std::conditional_t<
        N * 3 <= 16, unsigned short, std::conditional_t<
        N * 3 <= 32, unsigned int, std::conditional_t<
        N * 3 <= 64, unsigned long long, std::conditional_t<
        N <= 32, unsigned int, std::conditional_t<
        N <= 64, unsigned long long, void
    >>>>>>;

    uint_t x : N = 0;
    uint_t y : N = 0;
    uint_t z : N = 0;

    template< size_t M > explicit( M < N )
    constexpr operator uvec3< M >() const
    {
        return { x, y, z };
    }

    explicit( N > 31 ) constexpr operator ivec3() const
    {
        return { x, y, z };
    }

    uvec3& operator +=( ivec3 v )
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
};

template< size_t N >
size_t encode( uvec3< N > v )
{
    size_t out = 0;
    for ( int i = 0; i < N; ++i )
    {
        (out |= (v.x >> i) & 1) <<= 1;
        (out |= (v.y >> i) & 1) <<= 1;
        (out |= (v.z >> i) & 1) <<= 1;
    }
    return out >> 1;
}

template< size_t N >
uvec3< N > decode( size_t s )
{
    uvec3< N > out { 0, 0, 0 };
    for ( int i = 0; i < N; ++i )
    {
        out.z |= ((s >> i*3 + 0) & 1) << i;
        out.y |= ((s >> i*3 + 1) & 1) << i;
        out.x |= ((s >> i*3 + 2) & 1) << i;
    }
    return out;
}

// 3D boolean vector
struct bvec3
{
    bool x : 1 = 0;
    bool y : 1 = 0;
    bool z : 1 = 0;

    constexpr operator ivec3() const { return { x, y, z }; }
    template< size_t N >
    constexpr operator uvec3< N >() const { return { x, y, z }; }
    constexpr explicit operator int() const { return 1*x + 2*y + 4*z; }
    constexpr bool operator ==( const bvec3& v ) const { return x == v.x && y == v.y && z == v.z; }
};

constexpr ivec3 operator +( ivec3 a, ivec3 b )
{
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

constexpr ivec3 operator -( ivec3 a, ivec3 b )
{
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

constexpr ivec3 operator *( ivec3 a, int b )
{
    return { a.x * b, a.y * b, a.z * b };
}

std::ostream& operator <<( std::ostream& os, ivec3 v )
{
    return os << "x:" << v.x << " y:" << v.y << " z:" << v.z;
}

constexpr ivec3& operator +=( ivec3& a, ivec3 b )
{
    return a = a + b;
}

// Smallest possible Hilbert curve
const bvec3 proto_curve[ 8 ]{
    { 0, 0, 0 },
    { 0, 1, 0 },
    { 1, 1, 0 },
    { 1, 0, 0 },
    { 1, 0, 1 },
    { 1, 1, 1 },
    { 0, 1, 1 },
    { 0, 0, 1 },
};

using rot_mapping = std::array< int, 8 >;

constexpr rot_mapping operator *( const rot_mapping& a, const rot_mapping& b )
{
    rot_mapping c;
    for ( int i = 0; i < 8; ++i )
        c[ i ] = b[ a[ i ] ];
    return c;
}

constexpr rot_mapping v000 { 0, 7, 4, 3, 2, 5, 6, 1 };
constexpr rot_mapping v010 { 0, 1, 6, 7, 4, 5, 2, 3 };
constexpr rot_mapping v110 { 0, 1, 2, 3, 4, 5, 6, 7 };
constexpr rot_mapping v100 { 6, 1, 0, 7, 4, 3, 2, 5 };
constexpr rot_mapping v101 { 2, 5, 4, 3, 0, 7, 6, 1 };
constexpr rot_mapping v111 { 0, 1, 2, 3, 4, 5, 6, 7 };
constexpr rot_mapping v011 { 4, 5, 2, 3, 0, 1, 6, 7 };
constexpr rot_mapping v001 { 6, 1, 2, 5, 4, 3, 0, 7 };

constexpr rot_mapping mapping_for( bvec3 coord )
{
    constexpr rot_mapping map[] {
        v000, v100, v010, v110, v001, v101, v011, v111,
    };
    return map[ int( coord ) ];
}

constexpr long long POW( int b, size_t n )
{
    long long p = 1;
    while ( n-- > 0 ) p *= b;
    return p;
}

template< size_t ORDER, size_t ROOT = ORDER >
struct hilbert_curve
{
    static_assert( ORDER > 0 );
    static constexpr int SIZE = POW( 2, ORDER );

    using vec_t = uvec3< ROOT >;

    using child_t = std::conditional_t< ORDER != 1,
        hilbert_curve< ORDER - 1, ROOT >, vec_t >;

    std::array< child_t, 8 > arr;

    hilbert_curve( nullptr_t ) {}

    hilbert_curve()
    {
        for ( int i = 0; i < 8; ++i )
            arr[ i ] = child_t( proto_curve[ i ] );
    }

    explicit hilbert_curve( bvec3 coord ) : hilbert_curve()
    {
        *this = rotate( mapping_for( coord ) );
        translate( coord * SIZE );
    }

    hilbert_curve rotate( rot_mapping map ) const
    {
        hilbert_curve c = nullptr;
        for ( int i = 0; i < 8; ++i )
            c.arr[ i ] = rotate_child( i, map );
        return c;
    }

    child_t rotate_child( int i, rot_mapping map ) const
    {
        child_t child = arr[ map[i] ];
        ivec3 diff = proto_curve[ i ] - proto_curve[ map[i] ];
        if constexpr ( ORDER != 1 ) {
            child = child.rotate( map );
            diff = diff * child_t::SIZE;
            child += diff;
        }
        return child;
    }

    void translate( ivec3 v ) { for ( vec_t& w : *this ) w += v; }
    void operator +=( ivec3 v ) { translate( v ); }

    constexpr int size() const { return POW( SIZE, 3 ); }
    constexpr vec_t& operator []( int i ) { return ((vec_t*)this)[ i ]; }
    constexpr ivec3 operator []( int i ) const { return ((vec_t*)this)[ i ]; }

    constexpr auto begin() { return (vec_t*) this; }
    constexpr auto begin() const { return (const vec_t*) this; }
    constexpr auto end() { return begin() + size(); }
    constexpr auto end() const { return begin() + size(); }
};

int magnitude( ivec3 v )
{
    using std::abs;
    return abs(v.x) + abs(v.y) + abs(v.z);
}

void test_hilbert()
{
    using std::cout; using std::endl;
    cout << "hilbert test:\n";

    hilbert_curve< 2 > hc;

    cout << hc[ 0 ] << endl;
    for ( int i = 1; i < std::min( hc.size(), 1 << 7 ); ++i )
    {
        if ( magnitude( hc[ i ] - hc[ i - 1 ] ) != 1 )
        //if ( magnitude( decode< 2 >( i ) - decode< 2 >( i - 1 ) ) != 1 )
            cout << "ERROR ";
        if ( i % 8 == 0 )
            cout << endl;
        //cout << decode< 2 >( i ) << endl;
        cout << hc[ i ] << endl;
    }
    cout << endl;
}
