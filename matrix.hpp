#pragma once
#include "vec3.hpp"
#include <array>
#include <cassert>
#include <iomanip>
#include <sstream>

template< typename T, size_t N >
struct vecref : std::array< std::reference_wrapper< T >, N >
{
    using array_t = std::array< std::reference_wrapper< T >, N >;

    vecref& operator =( const std::array< T, N >& a )
    {
        for ( size_t i = 0; i < N; ++i )
            (*this)[ i ] = a[ i ];
        return *this;
    }
};

template< size_t M, size_t N = M >
struct matrix
    : std::array< std::array< double, N >, M >
{
    static_assert( M * N != 0 );

    using row_t = std::array< double, N >;
    using base = std::array< row_t, M >;
    using base::base;
    using base::at;

    static constexpr bool SQUARE = M == N;

    matrix operator -() const
    {
        matrix m;
        for ( int y = 0; y < M; ++y )
        for ( int x = 0; x < N; ++x )
            m[y][x] = -at(y)[x];
        return m;
    }

    friend matrix operator +( matrix a, matrix b )
    {
        for ( int y = 0; y < M; ++y )
        for ( int x = 0; x < N; ++x )
            a[y][x] += b[y][x];
        return a;
    }

    friend matrix operator -( matrix a, matrix b )
    {
        for ( int y = 0; y < M; ++y )
        for ( int x = 0; x < N; ++x )
            a[y][x] -= b[y][x];
        return a;
    }

    friend matrix operator *( double s, matrix m )
    {
        for ( int y = 0; y < M; ++y )
        for ( int x = 0; x < N; ++x )
            m[y][x] *= s;
        return m;
    }

    friend matrix operator *( matrix m, double s )
    {
        return s * m;
    }

    friend matrix operator /( matrix m, double s )
    {
        return 1 / s * m;
    }

    matrix() = default;
    matrix( const matrix& ) = default;

    explicit matrix( double d ) requires (SQUARE) : matrix()
    {
        static_assert( SQUARE, "diagonal matrix must be square" );
        for ( int i = 0; i < M * N; i += N + 1 )
            ((double*)this)[ i ] = d;
    }

    matrix( std::initializer_list< double > il ) : matrix()
    {
        assert( il.size() == M * N );
        std::memcpy( this, il.begin(), sizeof( matrix ) );
        //for ( int i = 0; i < M * N; ++i )
        //    ((double*)this)[ i ] = il.begin()[ i ];
    }

    matrix& fill( double d )
    {
        for ( int i = 0; i < M * N; ++i )
            ((double*)this)[ i ] = d;
        return *this;
    }

    matrix< N, M > transpose() const
    {
        matrix< N, M > m;
        for ( int y = 0; y < M; ++y )
        for ( int x = 0; x < N; ++x )
            m[x][y] = at(y)[x];
        return m;
    }

    template< size_t Y1, size_t Y2, size_t X1, size_t X2 >
    matrix< Y2 - Y1, X2 - X1 > sub() const
    {
        static_assert( Y1 < Y2 && Y2 <= M );
        static_assert( X1 < X2 && X2 <= N );
        constexpr size_t Y = Y2 - Y1;
        constexpr size_t X = X2 - X1;

        matrix< Y, X > m;
        for ( int y = 0; y < Y; ++y )
        for ( int x = 0; x < X; ++x )
            m[y][x] = at(y + Y1)[x + X1];
        return m;
    }

    matrix< M-1, N-1 > minor( size_t Y, size_t X ) const requires (M > 1 == N > 1)
    {
        assert( Y < M && X < N );
        if constexpr ( M * N == 1 ) return {};

        matrix< M-1, N-1 > m;
        for ( int y = 0; y < M; ++y )
        for ( int x = 0; x < N; ++x )
        {
            if ( y == Y || x == X ) continue;
            int y_ = y - (y >= Y);
            int x_ = x - (x >= X);
            m[y_][x_] = at(y)[x];
        }
        return m;
    }

    template< size_t N > friend double det( const matrix< N >& );

    double determinant() const requires (SQUARE)
    {
        return det( *this );
    }

    matrix cofactor() const requires (SQUARE)
    {
        matrix m;
        for ( int y = 0; y < M; ++y )
        for ( int x = 0; x < N; ++x )
        {
            double cofactor = minor( y, x ).determinant();
            m[ y ][ x ] = (y + x & 1) ? -cofactor : cofactor;
        }
        return m;
    }

    matrix adjoint() const requires (SQUARE)
    {
        return cofactor().transpose();
    }

    matrix inverse() const
    {
        return adjoint() / determinant();
    }

    auto row( int y )
    {
        return get_row( y, std::make_index_sequence< N >() );
    }

    auto col( int x )
    {
        return get_col( x, std::make_index_sequence< M >() );
    }

private:
    template< size_t... X >
    vecref< double, N > get_row( int y, std::index_sequence< X... > ) 
    {
        return { at( y )[ X ]... };
    }

    template< size_t... Y >
    vecref< double, M > get_col( int x, std::index_sequence< Y... > )
    {
        return { at( Y )[ x ]... };
    }
};

template<>
struct matrix< 0, 0 >
{
    static constexpr bool SQUARE = true;
    matrix operator -() const { return {}; }
    friend matrix operator +( matrix, matrix ) { return {}; }
    friend matrix operator -( matrix, matrix ) { return {}; }
    friend matrix operator *( matrix, matrix ) { return {}; }
    friend matrix operator *( double, matrix ) { return {}; }
    friend matrix operator *( matrix, double ) { return {}; }
    friend matrix operator /( matrix, double ) { return {}; }
    matrix transpose() const { return {}; }
    matrix cofactor() const { return {}; }
    matrix adjoint() const { return {}; }
    matrix inverse() const { return {}; }
};

double det( const matrix< 0 >& m ) { return 1; }
double det( const matrix< 1 >& m ) { return m[0][0]; }
double det( const matrix< 2 >& m )
{
    return m[0][0] * m[1][1] - m[0][1] * m[1][0];
}

template< size_t N > requires (N > 2)
double det( const matrix< N >& m )
{
    double d = 0;
    for ( int i = 0; i < N; ++i )
    {
        double cofactor = det( m.minor( 0, i ) );
        d += m[ 0 ][ i ] * (i & 1 ? -cofactor : cofactor);
    }
    return d;
}


template< typename Array, size_t... I >
auto array_dot( Array&& a, Array&& b, std::index_sequence< I... > ) 
{
    return ((a[I] * b[I]) + ...);
}

template< typename T, size_t N >
T operator *( const vecref< T, N >& a, const vecref< T, N >& b )
{
    return array_dot( a, b, std::make_index_sequence< N >() );
}

template< typename T, size_t N,
    template< typename, size_t > typename ArrA,
    template< typename, size_t > typename ArrB
>
std::array< T, N > operator +( const ArrA< T, N >& a, const ArrB< T, N >& b )
{
    std::array< T, N > c;
    for ( int i = 0; i < N; ++i )
        c[ i ] = a[ i ] + b[ i ];
    return c;
}

template< typename T, size_t N, template< typename, size_t > typename Arr >
std::array< T, N > operator *( const Arr< T, N >& a, const T& b )
{
    std::array< T, N > c;
    for ( int i = 0; i < N; ++i )
        c[ i ] = a[ i ] * b;
    return c;
}

template< typename T, size_t N, template< typename, size_t > typename Arr >
std::array< T, N > operator *( const T& a, const Arr< T, N >& b )
{
    return b * a;
}

template< typename T, size_t N >
void swap( const vecref< T, N >& a, const vecref< T, N >& b )
{
    using std::swap;
    for ( int i = 0; i < N; ++i )
        swap( a[ i ].get(), b[ i ].get() );
}


template< size_t M, size_t N, size_t P >
matrix< M, P > operator *( const matrix< M, N >& a, const matrix< N, P >& b )
{
    matrix< M, P > m;
    for ( int y = 0; y < M; ++y )
    for ( int x = 0; x < P; ++x )
        m[y][x] = a.row(y) * b.col(x);
    return m;
}

template< size_t M, size_t N >
std::ostream& operator <<( std::ostream& os, const matrix< M, N >& m )
{
    if constexpr ( M * N == 0 ) return os << "[]\n";

    constexpr char TL = M > 1 ? char(218) : '[';
    constexpr char TR = M > 1 ? char(191) : ']';
    constexpr char BR = 217;
    constexpr char BL = 192;
    constexpr char SD = 179;

    size_t width[ N ] = { 0 };
    if constexpr ( M > 1 )
    for ( int y = 0; y < M; ++y )
    for ( int x = 0; x < N; ++x )
    {
        std::ostringstream oss;
        oss.flags( os.flags() );
        oss.precision( os.precision() );
        oss << m[ y ][ x ];
        if ( size_t len = oss.str().length(); len > width[ x ] )
            width[ x ] = len;
    }

    for ( int y = 0; y < M; ++y )
    {
        for ( int x = 0; x < N; ++x )
        {
            if ( x == 0 )
                os << (y == 0 ? TL : y == M-1 ? BL : SD);

            os << std::setw( width[ x ] ) << m[ y ][ x ];
            
            if ( x == N-1 )
                os << (y == 0 ? TR : y == M-1 ? BR : SD);
            else os << ' '; 
        }
        os << '\n';
    }
    return os;
}

void test_matrix()
{
    using namespace std;
    matrix< 3, 3 > m {
        1, 2, 3,
        0, 4, 5,
        1, 0, 6,
    };
    cout << m << m.cofactor() << m.inverse();
    cout << m;
    swap( m.row( 0 ), m.row( 1 ) );
    cout << m << matrix< 1, 2 > { 1, 2 };
}
