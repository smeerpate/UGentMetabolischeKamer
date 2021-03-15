#ifndef TEMPSENSOR_H
#define TEMPSENSOR_H

#include <stdint.h>

int TempSensor_CalculateTempCx10(int16_t iTcValueRaw, uint16_t uiChipTempValueRaw);
int TempSensor_CalculateChipTemp(uint16_t uiChipTempValueRaw);

#endif
