/**********************************************************************
* File: Main_6.c -- File for Lab 6
* Devices: TMS320F28x7x
* Author: C2000 Technical Training, Texas Instruments
**********************************************************************/

#include "Lab.h"                        // Main include file
#include "sevenSegment.h"
#include "TempSensor.h"
#include "ads1120.h"
#include "buttons.h"
#include "Delay.h"
#include "Spi.h"

//--- Macro's
#define DEBOUNCE_TIME       1 // 10ms
#define CONVERSION_TIME     6 // 60ms

//--- Global Variables
Uint16 DEBUG_TOGGLE = 1;                // Used for realtime mode investigation test
Uint16 SINE_ENABLE = 0;                 // Used for DAC waveform generation
Uint16 AdcBuf[ADC_BUF_LEN];             // ADC buffer allocation
Uint16 DacOffset;                       // DAC offset
Uint16 DacOutput;                       // DAC output

Uint16 LoopCount;
Uint16 mawAdcMeasurements[2] = { 0 }; // initialize global buffer for ADC measurements
int miSetValueDegCx10 = 10;
int miCurrTempDegCx10 = 0;

//--- Enums
enum eStates
{
    S_START_TC_MEASUREMENT, // TC: Thermocouple
    S_START_ITS_MEASUREMENT, // ITS: Internal Temperature Sensor
    S_ADC_BACKOFF, // ADC is supposed to be busy, leave it alone
    S_ACQUIRE_CONVERSION_RESULT,
    S_CHECK_ADC,
    S_UPDATE_DISP1,
    S_UPDATE_DISP2,
    S_CHECK_BUTTON
};

// --- Prototypes
void mainStateMachine(void);
void mainDelayMs(Uint16);


/**********************************************************************
* Function: main()
*
* Description: Main function for C28x workshop labs
**********************************************************************/
void main(void)
{
//--- CPU Initialization
	InitSysCtrl();						// Initialize the CPU (FILE: SysCtrl.c)
	InitGpio();							// Initialize the shared GPIO pins (FILE: Gpio.c)
	InitXbar();							// Initialize the input, output & ePWM X-Bar (FILE: Xbar.c)
	InitPieCtrl();						// Initialize and enable the PIE (FILE: PieCtrl.c)
	InitWatchdog();						// Initialize the Watchdog Timer (FILE: WatchDog.c)

//--- Peripheral Initialization
//    InitAdca();                         // Initialize the ADC-A (FILE: Adc.c)
//    InitDacb();                         // Initialize the DAC-B (File: Dac.c)
//    InitEPwm();                         // Initialize the EPwm (FILE: EPwm.c)

//--- init software components
	spi_init();
    sevenSeg_init();
    sevenSeg_clear(1);
    delay_ms(200);
    sevenSeg_writeDisco(1);
    sevenSeg_clear(2);
    sevenSeg_writeDisco(2);

    ads1120_init();
    buttons_init();

//--- Enable global interrupts
	asm(" CLRC INTM, DBGM");			// Enable global interrupts and realtime debug

//--- Main Loop
	while(1)							// endless loop - wait for an interrupt
	{
		//asm(" NOP");
	    DelayUs(10000);
	    mainStateMachine();
	}

} //end of main()


void mainStateMachine(void)
{
    static enum eStates eState = S_CHECK_ADC;//S_CHECK_ADC;
    static int iDebounceCnt = 0;
    static unsigned char bSmExecCnt = 0;
    static bool biTcAcqBusy = false;
    static unsigned char bTcAcqStartCnt = 0;
    static bool biItsAcqBusy = false;
    static unsigned char bItsAcqStartCnt = 0;
    int iBtnState = 0;

    switch(eState)
    {
        case S_CHECK_ADC:
            if(biTcAcqBusy)
            {
                if(bSmExecCnt >= (bTcAcqStartCnt + CONVERSION_TIME))
                {
                    // Thermocouple conversion is ready, read and save value and...
                    mawAdcMeasurements[0] = ads1120_getConversionResult();
                    biTcAcqBusy = false;
                    // ...start an ITS measurement
                    eState = S_START_ITS_MEASUREMENT;
                    //break;
                }
                else
                {
                    // busy acquiring TC value...
                    eState = S_UPDATE_DISP1;
                    //break;
                }
            }
            else
            {
                if(biItsAcqBusy)
                {
                    if(bSmExecCnt >= (bItsAcqStartCnt + CONVERSION_TIME))
                    {
                        // ITS conversion is ready, read and save value and...
                        mawAdcMeasurements[1] = ads1120_getConversionResult();
                        biItsAcqBusy = false;
                        // ...calculate temperature and start TC measurement now we have a recent TC and ITS value.
                        miCurrTempDegCx10 = TempSensor_CalculateTempCx10((int)mawAdcMeasurements[0], mawAdcMeasurements[1]);
                        eState = S_UPDATE_DISP1;
                        //break;
                    }
                    else
                    {
                        // busy acquiring ITS value...
                        eState = S_UPDATE_DISP1;
                        //break;
                    }
                }
                else
                {
                    // Not converting, start a conversion
                    eState = S_START_TC_MEASUREMENT;
                    //break;
                }
            }
        break;

        case S_START_TC_MEASUREMENT:
            ads1120_cfgChThermocouple();
            ads1120_startConversion();
            biTcAcqBusy = true;
            bTcAcqStartCnt = bSmExecCnt; // set start of conversion timestamp
            eState = S_UPDATE_DISP1;
        break;

        case S_START_ITS_MEASUREMENT:
            ads1120_cfgChInternalTempSensor();
            ads1120_startConversion();
            biItsAcqBusy = true;
            bItsAcqStartCnt = bSmExecCnt; // set start of conversion timestamp
            eState = S_UPDATE_DISP1;
        break;

        case S_UPDATE_DISP1:
            sevenSeg_writeTemp(miCurrTempDegCx10, 1);
            eState = S_UPDATE_DISP2;
        break;

        case S_UPDATE_DISP2:
            sevenSeg_writeTemp(miSetValueDegCx10, 2);
            eState = S_CHECK_BUTTON;
        break;

        case S_CHECK_BUTTON:
            iBtnState = buttons_checkPress();
            if(iBtnState == 0)
            {
                iDebounceCnt = 0;
            }
            else
            {
                if(iDebounceCnt >= DEBOUNCE_TIME+1)
                {
                    eState = S_CHECK_ADC;
                    break;
                }
                else
                {
                    iDebounceCnt += 1;
                    if(iDebounceCnt >= DEBOUNCE_TIME)
                    {
                        iDebounceCnt += 1;
                        switch(iBtnState)
                        {
                            case 1:
                                miSetValueDegCx10 += 1;
                                break;
                            case 2:
                                miSetValueDegCx10 -= 1;
                                break;
                            default:
                                break;
                        }
                        eState = S_CHECK_ADC;
                        break;
                    }
                    else
                    {
                        eState = S_CHECK_ADC;
                        break;
                    }
                }
            }
            eState = S_CHECK_ADC;
        break;

        default:
            eState = S_CHECK_ADC;
        break;
    }

    bSmExecCnt += 1;
}




/*** end of file *****************************************************/
