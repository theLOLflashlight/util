#pragma once
#include "integer.hpp"

union dissected_float32
{
    float value;
    struct {
        uint mantissa : 23;
        uint exponent : 8;
        uint sign : 1;
    };
};

union dissected_float64
{
    long double value;
    struct {
        ulong mantissa : 52;
        ulong exponent : 11;
        ulong sign : 1;
    };
};

template< uint Exponent, uint Mantissa = Exponent * 5 >
struct decimal
{
    static constexpr uint EXPONENT_BITS = Exponent;
    static constexpr uint MANTISSA_BITS = Mantissa;
    static constexpr uint TOTAL_BITS = 1 + EXPONENT_BITS + MANTISSA_BITS;

    static constexpr uint NUM_INTS = (TOTAL_BITS + 31) / 32;
    static constexpr uint INT_MASK = ~(TOTAL_BITS % 32 ? ~0 << TOTAL_BITS % 32 : 0);

    using exponent_t = integer< EXPONENT_BITS + 1, signed >;
    using mantissa_t = integer< MANTISSA_BITS + 1, unsigned >;

    std::array< uint, NUM_INTS > ints;

    constexpr decimal() : ints { 0 } {}

    decimal( const decimal& ) = default;

    uint sign() const
    {
        return (ints.back() >> (TOTAL_BITS - 1) % 32) & 1;
    }

    exponent_t exponent() const
    {
        static const exponent_t bias = (exponent_t( 1 ) << EXPONENT_BITS - 1) - 1;
        auto& tmp = reinterpret_cast< integer< TOTAL_BITS, unsigned >& >( *this );
        exponent_t exp = reinterpret_cast< exponent_t& >( tmp >> MANTISSA_BITS );
        exp.ints.back() &= ~(1 << (EXPONENT_BITS - 1) % 32);
        exp.extend_sign();
        return exp - bias;
    }

    mantissa_t mantissa() const
    {
        mantissa_t mant = reinterpret_cast< mantissa_t& >( *this );
        mant.ints.back() |= 1 << (MANTISSA_BITS - 1) % 32;
        //mant.ints.back() &= ~(sign() << MANTISSA_BITS % 32);
        mant.extend_sign();
        return mant;
    }
};
