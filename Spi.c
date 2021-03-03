#include "Lab.h"

#define SPI_CPU_FRQ     200E6
#define SPI_BRR         ((SPI_CPU_FRQ / 4) / 100E3) - 1


void spi_init(void)
{
    SpiaRegs.SPICCR.bit.SPISWRESET = 0;         // reset peripheral before configuration
    SpiaRegs.SPICCR.bit.CLKPOLARITY = 0;        // Clock polarity: rising edge
    SpiaRegs.SPICCR.bit.SPICHAR = 7;            // 8-bit characters
    SpiaRegs.SPICCR.bit.SPILBK = 0;             // Disable loopback

    SpiaRegs.SPICTL.bit.MASTER_SLAVE = 1;       // peripheral is SPI master
    SpiaRegs.SPICTL.bit.TALK = 1;               // enable transmission
    SpiaRegs.SPICTL.bit.CLK_PHASE = 0;          // clock phase = normal
    SpiaRegs.SPICTL.bit.SPIINTENA = 0;          // disable SPI interrupts

    SpiaRegs.SPIBRR.bit.SPI_BIT_RATE = SPI_BRR; // set bitrate

    SpiaRegs.SPIPRI.bit.FREE = 1;               // halting on breakpoint will not halt SPI transfer

    SpiaRegs.SPICCR.bit.SPISWRESET = 1;         // release peripheral from reset
}
