//#include "F28x_Project.h"
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