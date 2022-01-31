#include "console_color.hpp"
#include <Windows.h>

std::ostream& operator <<( std::ostream& os, ConsoleColorFormat color )
{
    DWORD console = 0;
    if ( &os == &std::cout ) console = STD_OUTPUT_HANDLE;
    else if ( &os == &std::cerr ) console = STD_ERROR_HANDLE;
    if ( console ) SetConsoleTextAttribute( GetStdHandle( console ), color.wAttributes );
    return os;
}
