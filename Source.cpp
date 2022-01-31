#include "hilbert.hpp"

#include "quat.hpp"
#include "matrix.hpp"

#include "si_units.hpp"

#include "geo.hpp"
#include "job_system.hpp"

#include "integer.hpp"
INIT_INTEGER_POWERS( 100 );

#include "bitview.hpp"

using std::cout;
using std::endl;

void test()
{
    integer< 50 > a = 0;
    integer< 100 > b = 0;
    integer< 49, unsigned > c = 0;

    a += (integer< 50 >) b;
    b += a;
    b += (integer< 50, signed >) c;
    a += (integer< 49, unsigned >) b;

    auto d = a + b;
    auto e = a + c;
    auto f = a < b;
    auto g = b < a;
    auto h = a <= c;
}

int main()
{
    test_hilbert();

    test_unit( std::cout );

    test_quat();
    test_matrix();

    test_job();
    test_geo();
    main2();

    test();
    constexpr uint BITS = 100;

    integer< BITS, unsigned > z( "-1" );
    integer< BITS > a = 0;
    integer< BITS > b = 1;
    integer< BITS > c = -1;
    integer< BITS > d = -2;
    integer< BITS > big = INT_MAX;

    #define REPORT( EXPR ) cout << #EXPR " = " << (EXPR) << endl
    REPORT( z );
    REPORT( bits( z ) );
    REPORT( bits( a ) );
    REPORT( bits( b ) );
    REPORT( bits( c ) );
    REPORT( bits( d ) );
    REPORT( a );
    REPORT( b );
    REPORT( c );
    REPORT( a + b );
    REPORT( a - b );
    REPORT( -a );
    REPORT( -b );
    REPORT( -c );
    REPORT( b + -9999 );
    REPORT( b + c );
    REPORT( bits( b + c ) );
    REPORT( b + b );
    REPORT( c + c );
    REPORT( b * c );
    REPORT( c * b );
    REPORT( b * b );
    REPORT( c * c );
    REPORT( big ); REPORT( bits( big ) );
    REPORT( big * 2 ); REPORT( bits( big * 2 ) );
    REPORT( big * big ); REPORT( bits( big * big ) );
    REPORT( big * big * big ); REPORT( bits( big * big * big ) );
    REPORT( integer< BITS >( "123456789123456789123456789" ) );
    REPORT( integer< BITS >( "-123456789" ) );
    REPORT( integer< BITS >( "+123456789" ) );

    a += 1;
    a = a + 1;
}
