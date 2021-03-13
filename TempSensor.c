#include "TempSensor.h"

#define ADC_CONVERSIONCONSTANT  1.953125 // µV/LSB
#define ADC_INTTEMPCONVCONSTANT 0.03125 // °C/LSB
#define TC_LUTENTRIES           9


int TempSensor_CalculateChipTemp(uint16_t uiChipTempValueRaw);
int TempSensor_KTcVoltToTemp(long fVoltageMicroV);
long TempSensor_KTcTempToVolt(int iTempCx100);


long malKTcVoltLUT[] = {-1527, -1156, -778, -392,        // in µV, -40,-30,-20,-10°C
                      0, 397, 798, 1203, 1612,          // in µV, 0,10,20,30,40°C
                      };

int maiKTcTempLUT[] = {-4000, -3000, -2000, -1000,
                      0, 1000, 2000, 3000, 4000,
                      };

// All temperatueres in °C x 100 (eg: 1.23°C --> 123) (int format)
// All voltages in µV (long format)

int TempSensor_CalculateTempCx10(int16_t iTcValueRaw, uint16_t uiChipTempValueRaw)
{
    int iReturnValue = 0;

    int iChipTemp = TempSensor_CalculateChipTemp(uiChipTempValueRaw);
    long lCjVoltage = TempSensor_KTcTempToVolt(iChipTemp);
    long lTcVoltage = (long)((float)iTcValueRaw * ADC_CONVERSIONCONSTANT);

    iReturnValue = TempSensor_KTcVoltToTemp(lCjVoltage + lTcVoltage);
    iReturnValue = iReturnValue / 10;

    return iReturnValue;
}

int TempSensor_CalculateChipTemp(uint16_t uiChipTempValueRaw)
{
    float fTemp = 0.0;

    // Raw value is 14 bit Left justified
    uiChipTempValueRaw = uiChipTempValueRaw >> 2;

    if((uiChipTempValueRaw & 0x2000) != 0)
    {
        // value is negative, stretch sign bits
        uiChipTempValueRaw |= 0xc000;
    }

    fTemp = (float)uiChipTempValueRaw * ADC_INTTEMPCONVCONSTANT;
    return (int)(fTemp * 100);
}

int TempSensor_KTcVoltToTemp(long lVoltageMicroV)
{
    int iLUTIdxCnt = 0;
    int iTemp = 0;

    for(iLUTIdxCnt = 0; iLUTIdxCnt < TC_LUTENTRIES; iLUTIdxCnt++)
    {
        if(malKTcVoltLUT[iLUTIdxCnt] > lVoltageMicroV)
        {
            break;
        }
    }

    if(iLUTIdxCnt >= TC_LUTENTRIES)
    {
        iTemp = maiKTcTempLUT[TC_LUTENTRIES-1];
    }
    else
    {
        if(iLUTIdxCnt < 1)
        {
            iTemp = maiKTcTempLUT[0];
        }
        else
        {
            // interpolate
            // Slope: in °Cx100/µV
            int iDeltaV = (int)((malKTcVoltLUT[iLUTIdxCnt] - malKTcVoltLUT[iLUTIdxCnt-1]));
            int iDeltaT = (maiKTcTempLUT[iLUTIdxCnt] - maiKTcTempLUT[iLUTIdxCnt-1]);
            float fSlope = (float)iDeltaT / (float)iDeltaV;
            long lVoltOffset = lVoltageMicroV - malKTcVoltLUT[iLUTIdxCnt-1];
            iTemp = (int)((float)maiKTcTempLUT[iLUTIdxCnt-1] + (float)lVoltOffset * fSlope);
        }
    }

    return iTemp;
}

long TempSensor_KTcTempToVolt(int iTempCx100)
{
    int iLUTIdxCnt = 0;
    long lVolt = 0;

    for(iLUTIdxCnt = 0; iLUTIdxCnt < TC_LUTENTRIES; iLUTIdxCnt++)
    {
        if(iTempCx100 < maiKTcTempLUT[iLUTIdxCnt])
        {
            break;
        }
    }

    if(iLUTIdxCnt >= TC_LUTENTRIES)
    {
        lVolt = malKTcVoltLUT[TC_LUTENTRIES-1];
    }
    else
    {
        if(iLUTIdxCnt < 1)
        {
            lVolt = malKTcVoltLUT[0];
        }
        else
        {
            // interpolate
            // Slope: in µV/°Cx100
            int iDeltaV = (int)((malKTcVoltLUT[iLUTIdxCnt] - malKTcVoltLUT[iLUTIdxCnt-1]));
            int iDeltaT = (maiKTcTempLUT[iLUTIdxCnt] - maiKTcTempLUT[iLUTIdxCnt-1]);
            float fSlope = (float)iDeltaV / (float)iDeltaT;
            int iTempOffset = iTempCx100 - maiKTcTempLUT[iLUTIdxCnt-1];
            lVolt = (long)((float)malKTcVoltLUT[iLUTIdxCnt-1] + (float)iTempOffset * fSlope);
        }
    }

    return lVolt;
}
