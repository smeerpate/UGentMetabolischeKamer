#include "buttons.h"
#include "Lab.h"

#define BUTTONS_UPPIN               25
#define BUTTONS_DOWNPIN             26
#define BUTTONS_XINT1GPIO           BUTTONS_UPPIN
#define BUTTONS_XINT2GPIO           BUTTONS_DOWNPIN
#define BUTTONS_ACTIVELEVEL         0 // Active low

#define BUTTONS_DEBOUNCETIMEUSECS   600
#define BUTTONS_SYSCLOCKOUTRATEMHZ  100
#define BUTTONS_DEBOUNCETIMETICKS   BUTTONS_DEBOUNCETIMEUSECS * BUTTONS_SYSCLOCKOUTRATEMHZ 

/*
    int buttons_init()
    ------------------
    Inturrups should be disabled at CPU level before entering this function.
    Use DINT to do so.
    In the current implementation, this is done in void InitPieCtrl(void).
    (In Common/F2837xD_PieCtrl.c)
    Make sure to call void InitPieCtrl(void) before calling this one.
*/
int buttons_init()
{
    // connect GPIO to CPU interrupt lines via input XBAR
    // UP button
    //GPIO_SetupPinMux(BUTTONS_UPPIN, GPIO_MUX_CPU1, 0x00);
    //GPIO_SetupPinOptions(BUTTONS_UPPIN, GPIO_INPUT, GPIO_PULLUP);
    //GpioCtrlRegs.GPAPUD.bit.GPIO25 = 1;  //enable pull-up
    //GPIO_SetupXINT1Gpio(BUTTONS_UPPIN); // Make GPIO the input source for XINT1
    // Down button
    //GPIO_SetupPinMux(BUTTONS_DOWNPIN, GPIO_MUX_CPU1, 0x00);
    //GPIO_SetupPinOptions(BUTTONS_DOWNPIN, GPIO_INPUT, GPIO_PULLUP);
    //GpioCtrlRegs.GPAPUD.bit.GPIO26 = 1;  //enable pull-up
    //GPIO_SetupXINT2Gpio(BUTTONS_DOWNPIN); // Make GPIO the input source for XINT2
    
    // Instructions from Texas Instruments SPRUHM8H, p.94
    // 1. Disable interrupts globally (DINT). Done in main.c
    // 2. enable the ePIE (PIECTRL.ENPIE)
    //PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    
    // 3. init ISR vector table. Done in main.c
    // 4. set appropriate PIEIERx bits
    // Enable interrupts from GROUP1 (each group has 16 channels INTx1..INTx16) see p.96, table 3-2
    //PieCtrlRegs.PIEIER1.bit.INTx4 = 1; // enable channel 4 from group 1 = XINT1
    //PieCtrlRegs.PIEIER1.bit.INTx5 = 1; // enable channel 5 from group 1 = XINT2
    
    // 5. set CPU IER bits for PIE group containing enabled interrupts
    // I.e. Group 1.
    //IER |= 0x0001;
    
    // 6. (configure and) enable peripheral interrupt
    //XintRegs.XINT1CR.bit.POLARITY = 0; // falling edge
    //XintRegs.XINT2CR.bit.POLARITY = 0; // falling edge
    //XintRegs.XINT1CR.bit.ENABLE = 1;
    //XintRegs.XINT2CR.bit.ENABLE = 1;
    
    // 7. Enable interrups globally (EINT)
    // Done in main.c after all init
    return 0;
}


/*
    int buttons_checkPress()
    ------------------------
    Returns:
    0: if no button pressed
    1: button to XINT1 is pressed
    2: button to XINT 2 is pressed
    3: both buttons to XINT1 & to XINT2 pressed
*/
int buttons_checkPress()
{
    int iRetValue = 0;
    if(GpioDataRegs.GPADAT.bit.GPIO25 == BUTTONS_ACTIVELEVEL)
    {
        //if(XintRegs.XINT1CTR > BUTTONS_DEBOUNCETIMETICKS)
        //{
            iRetValue |= 0x1;
        //}
    }
    
    if(GpioDataRegs.GPADAT.bit.GPIO26 == BUTTONS_ACTIVELEVEL)
    {
        //if(XintRegs.XINT2CTR > BUTTONS_DEBOUNCETIMETICKS)
        //{
            iRetValue |= 0x2;
        //}
    }
    
    if(GpioDataRegs.GPADAT.bit.GPIO27 == BUTTONS_ACTIVELEVEL)
    {
        //if(XintRegs.XINT2CTR > BUTTONS_DEBOUNCETIMETICKS)
        //{
            iRetValue |= 0x4;
        //}
    }

    return iRetValue;
}
