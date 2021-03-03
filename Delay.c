#include "Lab.h"

void delay_ms(Uint16 uiNms)
{
    Uint16 i;
    for(i = 0; i <= uiNms; i += 1)
    {
        DelayUs(1000);
    }
}
