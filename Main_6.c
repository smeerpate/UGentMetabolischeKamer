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
#include "plant.h"

//--- Macro's
#define BUILD_INFO              14
#define DEBOUNCE_TIME           1 // 10ms
#define CONVERSION_TIME         6 // 60ms
#define SETTEMPCX10_MIN         -300 // deg C * 10 (e.g. -300 => -30 deg C)
#define SETTEMPCX10_MAX         300 // deg C *10
#define CONTROL_DEBOUNCE_TIME   100 // 1000ms

//--- Global Variables
Uint16 DEBUG_TOGGLE = 1;                // Used for realtime mode investigation test
Uint16 SINE_ENABLE = 0;                 // Used for DAC waveform generation
Uint16 AdcBuf[ADC_BUF_LEN];             // ADC buffer allocation
Uint16 DacOffset;                       // DAC offset
Uint16 DacOutput;                       // DAC output

Uint16 LoopCount;
Uint16 mawAdcMeasurements[2] = { 0 }; // initialize global buffer for ADC measurements
int miSetValueDegCx10 = 150;
int miCurrTempDegCx10 = 0;
int miSettingTimeTempDeltaCx10 = 0;
int miContolDeadBandCx10 = 10; // deg C * 10 (e.g. -300 => -30 deg C), temperature used while controlling the plant
float mfDeltaFactor = 0.2;
int miMinDeadBandDegCx10 = 2; // deg C * 10 (e.g. -300 => -30 deg C)
bool mbiActive = false;

//--- Enums
enum eStates
{
    S_START_TC_MEASUREMENT, // TC: Thermocouple
    S_START_ITS_MEASUREMENT, // ITS: Internal Temperature Sensor
    S_CHECK_ADC,
    S_CONTROL,
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


	// Section secureRamFuncs contains user defined code that runs from CSM secured RAM
	memcpy(&secureRamFuncs_runstart, &secureRamFuncs_loadstart, (Uint32)&secureRamFuncs_loadsize);
	// Copy code from flash
	InitFlash();

//--- Peripheral Initialization
//    InitAdca();                         // Initialize the ADC-A (FILE: Adc.c)
//    InitDacb();                         // Initialize the DAC-B (File: Dac.c)
//    InitEPwm();                         // Initialize the EPwm (FILE: EPwm.c)

//--- init software components
	spi_init();
    sevenSeg_init();
    sevenSeg_clear(1);
    //delay_ms(200);
    //sevenSeg_writeDisco(1);
    sevenSeg_clear(2);
    //sevenSeg_writeDisco(2);
    sevenSeg_writeTemp(BUILD_INFO, 1); // show software version number xx.x
    delay_ms(2000);


    ads1120_init();
    buttons_init();

    plant_refrigirate(false);

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

int mainAbs(int x)
{
    return ( x<0 ) ? -x : x;
}


void mainStateMachine(void)
{
    static enum eStates eState = S_CHECK_ADC;//S_CHECK_ADC;
    static int iDebounceCnt = 0;
    static int iControlDebounceCnt = 0;
    static unsigned char bSmExecCnt = 0;
    static bool biTcAcqBusy = false;
    static bool biItsAcqBusy = false;
    int iBtnState = 0;

    switch(eState)
    {
        case S_CHECK_ADC:
            if(biTcAcqBusy)
            {
                if(bSmExecCnt >= CONVERSION_TIME)
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
                    if(bSmExecCnt >= CONVERSION_TIME)
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
            bSmExecCnt = 0; // set start of conversion timestamp
            eState = S_UPDATE_DISP1;
        break;

        case S_START_ITS_MEASUREMENT:
            ads1120_cfgChInternalTempSensor();
            ads1120_startConversion();
            biItsAcqBusy = true;
            bSmExecCnt = 0; // set start of conversion timestamp
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
                    //eState = S_CHECK_ADC;
                    //break;
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
                                if(miSetValueDegCx10 < SETTEMPCX10_MAX)
                                {
                                    miSetValueDegCx10 += 1;
                                }
                                miSettingTimeTempDeltaCx10 = miSetValueDegCx10 - miCurrTempDegCx10; // update the setting time temperature difference
                                miContolDeadBandCx10 = (int)((float)miSettingTimeTempDeltaCx10 * mfDeltaFactor); // Dead band calculation
                                if(miContolDeadBandCx10 < miMinDeadBandDegCx10)
                                {
                                    miContolDeadBandCx10 = miMinDeadBandDegCx10;
                                }
                                break;
                            case 2:
                                if(miSetValueDegCx10 > SETTEMPCX10_MIN)
                                {
                                    miSetValueDegCx10 -= 1;
                                }
                                miSettingTimeTempDeltaCx10 = miSetValueDegCx10 - miCurrTempDegCx10; // update the setting time temperature difference
                                miContolDeadBandCx10 = (int)((float)miSettingTimeTempDeltaCx10 * mfDeltaFactor);
                                if(miContolDeadBandCx10 < miMinDeadBandDegCx10)
                                {
                                    miContolDeadBandCx10 = miMinDeadBandDegCx10;
                                }
                                break;
                            case 4:
                                if(mbiActive == true)
                                {
                                    mbiActive = false;
                                }
                                else
                                {
                                    mbiActive = true;
                                    miSettingTimeTempDeltaCx10 = miSetValueDegCx10 - miCurrTempDegCx10; // update the setting time temperature difference
                                    miContolDeadBandCx10 = (int)((float)miSettingTimeTempDeltaCx10 * mfDeltaFactor);
                                    if(miContolDeadBandCx10 < miMinDeadBandDegCx10)
                                    {
                                        miContolDeadBandCx10 = miMinDeadBandDegCx10;
                                    }
                                }
                            default:
                                break;
                        }
                        //eState = S_CHECK_ADC;
                        //break;
                    }
                    else
                    {
                        //eState = S_CHECK_ADC;
                        //break;
                    }
                }
            }
            eState = S_CONTROL;
        break;

        case S_CONTROL:
            if(mbiActive)
            {
                GpioDataRegs.GPESET.bit.LED_ACTIVE = 1; // LED is fixed to GND

                if(miCurrTempDegCx10 > (miSetValueDegCx10 + mainAbs(miContolDeadBandCx10)))
                {
                    if(iControlDebounceCnt >= CONTROL_DEBOUNCE_TIME)
                    {
                        // Cool down
                        plant_refrigirate(true);
                        plant_heat(false);
                        GpioDataRegs.GPATOGGLE.bit.LED_BLUE = 1;
                        GpioDataRegs.GPBSET.bit.LED_RED = 1; // LED is fixed to VCC
                        GpioDataRegs.GPCTOGGLE.bit.LED_COOL = 1;
                        GpioDataRegs.GPBCLEAR.bit.LED_HEAT = 1; // LED is fixed to GND
                    }
                    else
                    {
                        iControlDebounceCnt += 1;
                    }
                }
                else
                {
                    if(miCurrTempDegCx10 < (miSetValueDegCx10 - mainAbs(miContolDeadBandCx10)))
                    {
                        if(iControlDebounceCnt >= CONTROL_DEBOUNCE_TIME)
                        {
                            // warm up
                            plant_heat(true);
                            plant_refrigirate(false);
                            GpioDataRegs.GPBTOGGLE.bit.LED_RED = 1;
                            GpioDataRegs.GPASET.bit.LED_BLUE = 1;
                            GpioDataRegs.GPBTOGGLE.bit.LED_HEAT = 1;
                            GpioDataRegs.GPCCLEAR.bit.LED_COOL = 1;
                        }
                        else
                        {
                            iControlDebounceCnt += 1;
                        }
                    }
                    else
                    {
                        // idle
                        plant_heat(false);
                        plant_refrigirate(false);
                        GpioDataRegs.GPBTOGGLE.bit.LED_RED = 1;
                        GpioDataRegs.GPATOGGLE.bit.LED_BLUE = 1;
                        GpioDataRegs.GPCCLEAR.bit.LED_COOL = 1;
                        GpioDataRegs.GPBCLEAR.bit.LED_HEAT = 1;
                        iControlDebounceCnt = 0;
                    }
                }
            }
            else
            {
                plant_heat(false);
                plant_refrigirate(false);
                GpioDataRegs.GPASET.bit.LED_BLUE = 1; // LED is fixed to VCC
                GpioDataRegs.GPBSET.bit.LED_RED = 1; // LED is fixed to VCC
                GpioDataRegs.GPCCLEAR.bit.LED_COOL = 1; // LED is fixed to GND
                GpioDataRegs.GPBCLEAR.bit.LED_HEAT = 1; // LED is fixed to GND
                GpioDataRegs.GPECLEAR.bit.LED_ACTIVE = 1; // LED is fixed to GND
                iControlDebounceCnt = CONTROL_DEBOUNCE_TIME; // don't debounce when activating
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
