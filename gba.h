#pragma once
#include <array>
#include <stdint.h>

using std::array;

struct gba_color
{
    uint16_t red   : 5;
    uint16_t green : 5;
    uint16_t blue  : 5;
    uint16_t transparent : 1;
};

volatile auto& EW_RAM =      *reinterpret_cast< array< uint8_t, 256 * 1024 >* >( 0x2000000 );
volatile auto& IW_RAM =      *reinterpret_cast< array< uint8_t, 32 * 1024 >* > ( 0x3000000 );
volatile auto& IO_RAM =      *reinterpret_cast< array< uint8_t, 1024 >* >      ( 0x4000000 );
volatile auto& PALETTE_RAM = *reinterpret_cast< array< uint8_t, 1024 >* >      ( 0x5000000 );
volatile auto& V_RAM =       *reinterpret_cast< array< uint8_t, 0x20000 >* >   ( 0x6000000 );
volatile auto& OA_RAM =      *reinterpret_cast< array< uint8_t, 1024 >* >      ( 0x7000000 );
const volatile auto& ROM =   *reinterpret_cast< array< uint8_t, 0 >* >         ( 0x8000000 );

enum class gfx_mode : uint8_t {};

namespace GFX_0 {
    constexpr gfx_mode MODE { 0 };
}
namespace GFX_1 {
    constexpr gfx_mode MODE { 1 };
}
namespace GFX_2 {
    constexpr gfx_mode MODE { 2 };
}

namespace GFX_3 {
    constexpr gfx_mode MODE { 3 };
    volatile auto& SCREEN_BUFFER = *(array< array< gba_color, 240 >, 160 >*) 0x6000000;
}

namespace GFX_4 {
    constexpr gfx_mode MODE { 4 };
    volatile auto& SCREEN_BUFFER_A = *(array< array< uint8_t, 240 >, 160 >*) 0x6000000;
    volatile auto& SCREEN_BUFFER_B = *(array< array< uint8_t, 240 >, 160 >*) 0x600A000;
}

namespace GFX_5 {
    constexpr gfx_mode MODE { 5 };
    volatile auto& SCREEN_BUFFER_A = *(array< array< gba_color, 160 >, 128 >*) 0x6000000;
    volatile auto& SCREEN_BUFFER_B = *(array< array< gba_color, 160 >, 128 >*) 0x600A000;
}


using button_set = uint32_t;
const volatile auto& BUTTONS = *reinterpret_cast< button_set* >( 0x4000130 );

enum button_code {
    BUTTON_A, BUTTON_B, BUTTON_SELECT, BUTTON_START, BUTTON_RIGHT, BUTTON_LEFT, BUTTON_UP, BUTTON_DOWN, BUTTON_R, BUTTON_L
};

// returns true if the given button was or is pressed
constexpr bool is_down( button_code code, button_set buttons )
{
    return (buttons >> code) & 1;
}

// returns true is the given button IS pressed in the current button set 
// but ISN'T pressed in the previous button set
constexpr bool was_pressed( button_code code, button_set curr_buttons, button_set prev_buttons )
{
    return !is_down( code, prev_buttons ) && is_down( code, curr_buttons );
}

// returns true is the given button ISN'T pressed in the current button set 
// but IS pressed in the previous button set
constexpr bool was_released( button_code code, button_set curr_buttons, button_set prev_buttons )
{
    return is_down( code, prev_buttons ) && !is_down( code, curr_buttons );
}

int main()
{
    button_set prev_buttons = BUTTONS;
    for (;;)
    {
        button_set curr_buttons = BUTTONS;

        if ( was_pressed( BUTTON_START, curr_buttons, prev_buttons ) )
            ;

        prev_buttons = curr_buttons;
    }
}
