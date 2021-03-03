//#include "F28x_Project.h"
#include "ads1120.h"
#include "Lab.h"
#include "Delay.h"

#define ADS1120_CSPIN   GPIO19


void ads1120_spi_xmit(unsigned char a);

int ads1120_init()
{
    // SET CS logic high and wait
    asm(" EALLOW");
    GpioDataRegs.GPASET.bit.ADS1120_CSPIN = 1;
    DelayUs(100000);

    // configure the ADS1120
    GpioDataRegs.GPACLEAR.bit.ADS1120_CSPIN = 1;
    //DELAY_US(1E6);
    ads1120_spi_xmit(0x43); // write 4 bytes starting at configuration register 0 (WREG at register 0 = 0x40 + (number of bytes - 1) = 0x03)
    ads1120_spi_xmit(0x0A); // MUX[3:0] = 'b0000, GAIN[2:0] = 'b101, PGA_BYPASS = 'b0
    ads1120_spi_xmit(0x00); // DR[2:0] = 'b000, MODE[1:0] = 'b00, CM = 'b0, TS = 'b0, BCS = 'b0
    ads1120_spi_xmit(0x10); // VREF[1:0] = 'b00, 50/60[1:0] = 'b01, PSW = 'b0, IDAC[2:0] = 'b000
    ads1120_spi_xmit(0x00); // I1MUX[2:0] = 'b000, I2MUX[2:0] = 'b000, DRDYM = 'b0, 'b0
    GpioDataRegs.GPASET.bit.ADS1120_CSPIN = 1;
    asm(" EDIS");

    return 0;
}

/*
 * Function to acquire the thermocouple values
 * Wait for 100ms after calling this function.
 */
int ads1120_readThermocouple()
{
    Uint16 uwRData; // read data
    int iReturnValue;

    asm(" EALLOW");
    GpioDataRegs.GPACLEAR.bit.ADS1120_CSPIN = 1;
    ads1120_spi_xmit(0x44); // WREG starting at register 01h, one byte
    ads1120_spi_xmit(0x00); // Register 1: Data Rate=20SPS, Normal mode, Single shot conversion, Temp sensor and BCS disabled
    GpioDataRegs.GPASET.bit.ADS1120_CSPIN = 1;

    DelayUs(1000);
    GpioDataRegs.GPACLEAR.bit.ADS1120_CSPIN = 1;
    ads1120_spi_xmit(0x08); // Send START/SYNC command to start conversion
    DelayUs(51000); // Wait for conversion to be done
    //ads1120_spi_xmit(0x10); // RDATA command to read data from thermocouple
    ads1120_spi_xmit(0x00);  // clock out 8 bits for MSB
    uwRData = (SpiaRegs.SPIRXBUF & 0xFF) << 8;
    ads1120_spi_xmit(0x00); // clock out 8 bits for LSB
    uwRData |= SpiaRegs.SPIRXBUF & 0xFF;
    GpioDataRegs.GPASET.bit.ADS1120_CSPIN = 1;

    asm(" EDIS");

    iReturnValue = (int)uwRData;
    return iReturnValue;
}


/*
 * Function to acquire the internal temp sensor values.
 * Wait for 100ms after calling this function.
 */
int ads1120_readInternalTempSensor()
{
    Uint16 uwRData; // read data
    int iReturnValue;

    asm(" EALLOW");
    GpioDataRegs.GPACLEAR.bit.ADS1120_CSPIN = 1;
    ads1120_spi_xmit(0x44);
    ads1120_spi_xmit(0x02);
    GpioDataRegs.GPASET.bit.ADS1120_CSPIN = 1;

    DelayUs(1000);
    GpioDataRegs.GPACLEAR.bit.ADS1120_CSPIN = 1;
    ads1120_spi_xmit(0x08);  // Send START/SYNC command to start conversion
    DelayUs(51000);
    //ads1120_spi_xmit(0x10);  // RDATA command to read data from thermocouple
    ads1120_spi_xmit(0x00);  // clock out 8 bits for MSB
    uwRData = (SpiaRegs.SPIRXBUF & 0xFF) << 8;
    ads1120_spi_xmit(0x00); // clock out 8 bits for LSB
    uwRData |= SpiaRegs.SPIRXBUF & 0xFF;
    GpioDataRegs.GPASET.bit.ADS1120_CSPIN = 1;

    asm(" EDIS");

    iReturnValue = (int)uwRData;
    return iReturnValue;
}

void ads1120_cfgChThermocouple()
{
    asm(" EALLOW");
    GpioDataRegs.GPACLEAR.bit.ADS1120_CSPIN = 1;
    ads1120_spi_xmit(0x44); // WREG starting at register 01h, one byte
    ads1120_spi_xmit(0x00); // Register 1: Data Rate=20SPS, Normal mode, Single shot conversion, Temp sensor and BCS disabled
    GpioDataRegs.GPASET.bit.ADS1120_CSPIN = 1;
    asm(" EDIS");
}

void ads1120_cfgChInternalTempSensor()
{
    asm(" EALLOW");
    GpioDataRegs.GPACLEAR.bit.ADS1120_CSPIN = 1;
    ads1120_spi_xmit(0x44);
    ads1120_spi_xmit(0x02);
    GpioDataRegs.GPASET.bit.ADS1120_CSPIN = 1;
    asm(" EDIS");
}

void ads1120_startConversion()
{
    asm(" EALLOW");
    GpioDataRegs.GPACLEAR.bit.ADS1120_CSPIN = 1;
    ads1120_spi_xmit(0x08);  // Send START/SYNC command to start conversion
    GpioDataRegs.GPASET.bit.ADS1120_CSPIN = 1;
    asm(" EDIS");
}

int ads1120_getConversionResult()
{
    Uint16 uwRData; // read data
    int iReturnValue;

    asm(" EALLOW");
    GpioDataRegs.GPACLEAR.bit.ADS1120_CSPIN = 1;
    ads1120_spi_xmit(0x10);  // RDATA command to read data
    ads1120_spi_xmit(0x00);  // clock out 8 bits for MSB
    uwRData = (SpiaRegs.SPIRXBUF & 0xFF) << 8;
    ads1120_spi_xmit(0x00); // clock out 8 bits for LSB
    uwRData |= SpiaRegs.SPIRXBUF & 0xFF;
    GpioDataRegs.GPASET.bit.ADS1120_CSPIN = 1;
    asm(" EDIS");

    iReturnValue = (int)uwRData;
    return iReturnValue;
}

//
// ads1120_spi_xmit - Transmit value via SPI
//
void ads1120_spi_xmit(unsigned char a)
{
    SpiaRegs.SPITXBUF = (Uint16) a << 8;
    DelayUs(100);
}
