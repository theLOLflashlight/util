#pragma once
#include "vec3.hpp"

#include <iostream>
#include <cmath>
#include <vector>
#include <sstream>
#include <span>

using std::span;

struct vec2
{
    double x, y;

    vec2 operator -() const {
        return { -x, -y };
    }
    vec2 operator *( double a ) const {
        return { x * a, y * a };
    }
    vec2 operator /( double a ) const {
        return { x / a, y / a };
    }

    double length() const {
        return sqrt( x*x + y*y );
    }
    vec2 normalized() const {
        return *this / length();
    }

    friend vec2 operator +( vec2 a, vec2 b ) {
        return { a.x + b.x, a.y + b.y };
    }
    friend vec2 operator -( vec2 a, vec2 b ) {
        return { a.x - b.x, a.y - b.y };
    }

    double dot( vec2 v ) const {
        return x * v.x + y * v.y;
    }
};

vec2 lerp( double t, vec2 a, vec2 b )
{
    double u = 1 - t;
    return { a.x*u + b.x*t, a.y*u + b.y*t };
}

vec2 bezier( double t, vec2 a, vec2 b, vec2 c )
{
    return lerp( t, lerp( t, a, b ), lerp( t, b, c ) );
}

vec2 bezier( double t, span< vec2 > verts )
{
    switch ( verts.size() ) {
    case 0 : return {};
    case 1 : return verts[ 0 ];
    case 2 : return lerp( t, verts[ 0 ], verts[ 1 ] );
    case 3 : return bezier( t, verts[ 0 ], verts[ 1 ], verts[ 2 ] );
    }

    size_t numlines = verts.size() - 1;
    vec2* verts_ = stack_alloc( vec2, numlines );
    for ( int i = 0; i < numlines; ++i )
        verts_[ i ] = lerp( t, verts[ i ], verts[ i + 1 ] );

    return bezier( t, { verts_, numlines } );
}

struct Token
{
    bool is_num : 1;
    bool is_func : 1;
    union {
        int num;
        char func[ 4 ];
        char op;
    };

    Token() = default;
    Token( const Token& ) = default;

    explicit Token( int num )
        : is_num { true }
        , is_func { false }
        , num { num }
    {
    }

    explicit Token( char op )
        : is_num { false }
        , is_func { false }
        , op { op }
    {
    }

    explicit Token( span< const char, 4 > func )
        : is_num { false }
        , is_func { true }
        , func { func[ 0 ], func[ 1 ], func [ 2 ], func[ 3 ] }
    {
    }
};

std::ostream& operator <<( std::ostream& os, Token t )
{
    if ( t.is_num ) os << '#' << t.num;
    else if ( t.is_func ) os << t.func;
    else os << t.op;
    return os;
}

std::vector< Token > tokenize( std::string text )
{
    std::vector< Token > tokens;
    int n = 0, nend = text.length();
    bool digit = false;
    int func_i = 0;
    while ( n < nend )
    {
        char c = text[ n++ ];
        if ( func_i == 0 && c >= '0' && c <= '9' )
        {
            int num = c - '0';
            if ( !digit )
                tokens.emplace_back( num );
            else {
                tokens.back().num *= 10;
                tokens.back().num += num;
            }
            digit = true;
            continue;
        }
        digit = false;
        switch ( c )
        {
        case '+': case '-':
        case '*': case '/':
        case '(': case ')':
        case '[': case ']':
        case ',': case '.':
            tokens.emplace_back( c );
        case ' ': case '\t':
            func_i = 0;
            continue;
        case '\0': n = nend;
            continue;
        default: if ( (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') )
        {
            if ( func_i == 0 ) {
                char buff[ 4 ] = { c, 0, 0, 0 };
                tokens.emplace_back( buff );
            }
            else if ( func_i < 4 )
            {
                tokens.back().func[ func_i ] = c;
            }
            else goto error;
            ++func_i;
            continue;
        }
        }
    error:
        std::cout << "error";
        break;
    }
    return tokens;
}

#define NULL_NODE "null"

struct result
{
    double x, y, z;
    size_t mag : 2 = 1;

    result( const result& ) = default;

    result( double a = 0 ) : result( a, 0, 0, 1 ) {}
    result( double a, double b ) : result( a, b, 0, 2 ) {}
    result( double a, double b, double c ) : result( a, b, c, 3 ) {}

    result( vec2 v ) : result( v.x, v.y ) {}
    result( vec3 v ) : result( v.x, v.y, v.z ) {}

    explicit operator std::string() const
    {
        std::ostringstream oss;
        if ( mag != 1 ) oss << "(";
        if ( mag >= 1 ) oss << x;
        if ( mag >= 2 ) oss << ", " << y;
        if ( mag >= 3 ) oss << ", " << z;
        if ( mag != 1 ) oss << ")";
        return oss.str();
    }

    friend result operator +( result a, result b ) {
        return { a.x + b.x, a.y + b.y, a.z + b.z, std::max( a.mag, b.mag ) };
    }
    friend result operator -( result a, result b ) {
        return { a.x - b.x, a.y - b.y, a.z - b.z, std::max( a.mag, b.mag ) };
    }
    friend result operator *( result a, result b ) {
        return { a.x * b.x, a.y * b.y, a.z * b.z, std::max( a.mag, b.mag ) };
    }
    friend result operator /( result a, result b ) {
        return { a.x / b.x, a.y / b.y, a.z / b.z, std::max( a.mag, b.mag ) };
    }

    result operator -() const {
        return { -x, -y, -z, mag };
    }

    friend result abs( result a )
    {
        return { std::abs( a.x ), std::abs( a.y ), std::abs( a.z ), a.mag };
    }

    friend std::ostream& operator <<( std::ostream& os, result r )
    {
        return os << (std::string) r;
    }

private:
    result( double a, double b, double c, size_t mag )
        : x { a }, y { b }, z { c }, mag { mag } {}
};

struct Node
{
    virtual result eval() = 0;
    virtual std::string print() const = 0;
    virtual void free() {};
    virtual Node* append_stack( std::vector< Node** >& stack )
    {
        *stack.back() = this;
        stack.pop_back();
        return this;
    }
};

std::string print_node( Node* node )
{
    return node ? node->print() : NULL_NODE;
}

struct Number : Node
{
    double val;
    Number( double v = 0.0 ) : val { v } {}

    result eval() override { return val; }

    std::string print() const override
    {
        return (std::ostringstream{} << val).str();
    }
};

struct Vector : Node
{
    Node* val[ 3 ];
    size_t len = 1;

    Vector() = default;
    Vector( const Vector& ) = default;

    explicit Vector( size_t len ) : val { 0, 0, 0 }, len { len } {}

    result eval() override
    {
        result r;
        switch ( r.mag = len )
        {
        case 3 : r.z = val[ 2 ]->eval().x;
        case 2 : r.y = val[ 1 ]->eval().x;
        case 1 : r.x = val[ 0 ]->eval().x;
        }
        return r;
    }

    std::string print() const override
    {
        std::ostringstream oss;
        oss << "[" << print_node( val[ 0 ] );
        for ( size_t i = 1; i < len; ++i )
            oss << ", " << print_node( val[ i ] );
        oss << "]";
        return oss.str();
    }

    void free() override
    {
        for ( Node* node : std::span( val, len ) )
            if ( node ) {
                node->free();
                delete node;
            }
    }

    Vector* append_stack( std::vector< Node** >& stack ) override
    {
        Node::append_stack( stack );
        for ( int i = len - 1; i >= 0; --i )
            stack.push_back( &val[ i ] );
        return this;
    }
};

template< size_t Arity > requires (Arity > 0)
struct Function : Node
{
    Node* args[ Arity ] { nullptr };

    virtual std::string print() const
    {
        std::ostringstream oss;
        oss << "(" << print_node( args[ 0 ] );
        for ( int i = 1; i < (int) Arity; ++i )
            oss << ", " << print_node( args[ i ] );
        oss << ")";
        return oss.str();
    }

    void free() override
    {
        for ( Node* node : args )
            if ( node ) {
                node->free();
                delete node;
            }
    }

    Function* append_stack( std::vector< Node** >& stack ) override
    {
        Node::append_stack( stack );
        for ( int i = Arity - 1; i >= 0; --i )
            stack.push_back( &args[ i ] );
        return this;
    }
};

struct Negate : Function< 1 >
{
    result eval() override
    {
        return -args[ 0 ]->eval();
    }
    
    std::string print() const override
    {
        return "-" + print_node( args[ 0 ] );
    }
};

struct Absolute : Function< 1 >
{
    result eval() override
    {
        return abs( args[ 0 ]->eval() );
    }
    
    std::string print() const override
    {
        return "|" + print_node( args[ 0 ] ) + "|";
    }
};


#define DEFINE_BINOP( NAME, TEXT, OP )                 \
struct NAME : Function< 2 > {                          \
    result eval() override {                           \
        return args[ 0 ]->eval() OP args[ 1 ]->eval(); \
    }                                                  \
    std::string print() const override {               \
        return #TEXT + Function< 2 >::print();         \
    }                                                  \
};

DEFINE_BINOP( Multiply, mul, * );
DEFINE_BINOP( Divide, div, / );
DEFINE_BINOP( Add, add, + );
DEFINE_BINOP( Subtract, sub, - );

Node* parse( const std::vector< Token >& toks )
{
    Node* root = nullptr;
    std::vector< Node** > stack { &root };

    for ( Node* op = nullptr; Token tok : toks )
    {
        if ( tok.is_num )
        {
            op = new Number { (double) tok.num };
        }
        else if ( tok.is_func )
        {
            constexpr const char* fn_name[] { "neg", "abs", "v1", "v2", "v3" };
            constexpr Node* (*fn_new[ std::size( fn_name ) ])()
            {
                []() -> Node* { return new Negate; },
                []() -> Node* { return new Absolute; },
                []() -> Node* { return new Vector( 1 ); },
                []() -> Node* { return new Vector( 2 ); },
                []() -> Node* { return new Vector( 3 ); },
            };

            for ( int i = 0; i < std::size( fn_name ); ++i )
            {
                if ( !std::strncmp( tok.func, fn_name[ i ], 4 ) )
                {
                    op = fn_new[ i ]();
                    break;
                }
            }
        }
        else switch ( tok.op )
        {
            case '*' : op = new Multiply; break;
            case '/' : op = new Divide; break;
            case '+' : op = new Add; break;
            case '-' : op = new Subtract; break;
        }

        op->append_stack( stack );
    }

    return root;
}

int test_geo()
{
    auto v = tokenize( "abs * v1 5 * 11 neg 5" );
    for ( auto& t : v )
        std::cout << t << ' ';
    std::cout << std::endl;

    Node* ast = parse( v );
    std::cout << ast->print() << " = " << ast->eval();

    std::cout << std::endl;
    return 0;
}

struct line2
{
    vec2 a, b;
    line2 operator -() const { return { b, a }; }
    vec2 operator []( double t ) const {
        return lerp( t, a, b );
    }
};

struct tri2
{
    vec2 a, b, c;
    line2 A() const { return { b, c }; }
    line2 B() const { return { c, a }; }
    line2 C() const { return { a, b }; }
    line2 ab() const { return C(); }
    line2 bc() const { return A(); }
    line2 ca() const { return B(); }
};

struct line3
{
    vec3 a, b;
    line3 operator -() const { return { b, a }; }
    vec3 operator []( double t ) const {
        return lerp( a, b, t );
    }
};

struct tri3
{
    vec3 a, b, c;
    line3 A() const { return { b, c }; }
    line3 B() const { return { c, a }; }
    line3 C() const { return { a, b }; }
    line3 ab() const { return C(); }
    line3 bc() const { return A(); }
    line3 ca() const { return B(); }
};

struct poly_line2 : std::vector< vec2 >
{
    using std::vector< vec2 >::vector;

    vec2 operator []( double t ) const
    {
        int ti = (int) t;
        if ( t <= 0 ) return front();
        if ( ti + 1 >= size() ) return back();
        return lerp( t - ti, at( ti ), at( ti + 1 ) );
    }
};

struct poly_line3 : std::vector< vec3 >
{
    using std::vector< vec3 >::vector;

    vec3 operator []( double t ) const
    {
        int ti = (int) t;
        if ( t <= 0 ) return front();
        if ( ti + 1 >= size() ) return back();
        return lerp( at( ti ), at( ti + 1 ), t - ti );
    }
};
