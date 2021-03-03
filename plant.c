#include "plant.h"

int plant_refrigirate(bool biActive)
{
    int iReturnValue = 0;

    if(biActive)
        GpioDataRegs.GPASET.bit.PLANT_REFRIGITATECTRLPIN = 1;
    else
        GpioDataRegs.GPACLEAR.bit.PLANT_REFRIGITATECTRLPIN = 1;

    return iReturnValue;
}

int plant_heat(bool biActive)
{
    int iReturnValue = 0;

   if(biActive)
       GpioDataRegs.GPASET.bit.PLANT_HEATCTRLPIN = 1;
   else
       GpioDataRegs.GPACLEAR.bit.PLANT_HEATCTRLPIN = 1;

   return iReturnValue;
}
