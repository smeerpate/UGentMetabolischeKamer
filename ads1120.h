#ifndef ADS1120_H
#define ADS1120_H

int ads1120_init();
int ads1120_readThermocouple();
int ads1120_readInternalTempSensor();

void ads1120_cfgChThermocouple();
void ads1120_cfgChInternalTempSensor();
void ads1120_startConversion();
int ads1120_getConversionResult();

#endif
