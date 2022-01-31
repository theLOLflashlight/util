#pragma once
#include <iostream>
using WORD = unsigned short;

struct ConsoleColorFormat
{
    union {
        WORD wAttributes;
        struct {
            bool blue : 1;
            bool green : 1;
            bool red : 1;
            bool intensity : 1;

            bool bg_blue : 1;
            bool bg_green : 1;
            bool bg_red : 1;
            bool bg_intensity : 1;
        };
    };

    ConsoleColorFormat( const ConsoleColorFormat& ) = default;
    explicit ConsoleColorFormat( WORD wAttributes = 0b0000'0111 )
        : wAttributes { wAttributes }
    {
    }
};

std::ostream& operator <<( std::ostream& os, ConsoleColorFormat color );
