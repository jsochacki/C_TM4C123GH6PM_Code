/*
 * Synth_API_Control_Registers.c
 *
 * Author: Mason Edgar
 *
 */


#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_adc.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "inc/hw_ssi.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/debug.h"
#include "driverlib/adc.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/pwm.h"
#include "driverlib/ssi.h"
#include "Synth_API_Macro_Definitions.h"
#include "Synth_API_SPI_Setup.h"
#include "Synth_API_Control_Registers.h"



/***************************************************************************************************************************************/
/*                             Feedback Control Divider Registers
 *
 * The feedback divider N supports fractional division capability in the PLL feedback path. It consists
 * of an integer N divider of 16-bits, and a Fractional divider of 32-bits (FRAC) over 32-bits (MOD).
 * The 16 INT bits (Nint[15:0] in the Feedback Divider Control Registers) set the integer part of the
 * feedback division ratio. The 32 FRAC bits (Bit Nfrac[31:0] in the Feedback Divider Control Registers)
 * set the numerator of the fraction that goes into the Sigma Delta modulator. The 32 MOD bits
 * (Nmod[31:0] in the Feedback Divider Control Registers) set the denominator of the fraction that goes
 * into the Sigma Delta modulator.
 *
 *
 * FB Divider Integer Portion: Minimum divide ratio is 7d. Default is 31d
 *
 * FB Divider Fractional Portion: Nfrac is the numerator value of the fractional divide ratio. It is programmable from 0 to (Nmod - 1).
 *
 * FB divider Modulus Portion: Range is 2d - 4,294,967,295d. Default is 4,294,966,784d
 *
 */
/***************************************************************************************************************************************/



void SetFeedbackControlValues(unsigned short Nint, unsigned long Nfrac, unsigned long Nmod){

/*

	 Since the chip has 8-bit registers, the values for Nint, Nfrac, and Nmod are spread out across
	 multiple (consecutive) registers. The code below breaks each argument down into single-byte
	 pieces and sends them to their appropriate register.


                               Feedback Control Register Map (ADDR is in hex)

  +------------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
  |            |           |           |           |           |           |           |           |           |
  |    ADDR    |    D7     |    D6     |    D5     |    D4     |    D3     |    D2     |    D1     |    D0     |
  |            |           |           |           |           |           |           |           |           |
  +------------------------------------------------------------------------------------------------------------+
  |            |           |           |           |           |           |           |           |           |
  |    0010    |  Nint<7>  |  Nint<6>  |  Nint<5>  |  Nint<4>  |  Nint<3>  |  Nint<2>  |  Nint<1>  |  Nint<0>  |
  |            |           |           |           |           |           |           |           |           |
  +------------------------------------------------------------------------------------------------------------+
  |            |           |           |           |           |           |           |           |           |
  |    0011    |  Nint<15> |  Nint<14> |  Nint<13> |  Nint<12> |  Nint<11> |  Nint<10> |  Nint<9>  |  Nint<8>  |
  |            |           |           |           |           |           |           |           |           |
  +------------------------------------------------------------------------------------------------------------+
  |            |           |           |           |           |           |           |           |           |
  |    0012    |  Nfrac<7> |  Nfrac<6> |  Nfrac<5> |  Nfrac<4> |  Nfrac<3> |  Nfrac<2> |  Nfrac<1> |  Nfrac<0> |
  |            |           |           |           |           |           |           |           |           |
  +------------------------------------------------------------------------------------------------------------+
  |            |           |           |           |           |           |           |           |           |
  |    0013    | Nfrac<15> | Nfrac<14> | Nfrac<13> | Nfrac<12> | Nfrac<11> | Nfrac<10> | Nfrac<9>  | Nfrac<8>  |
  |            |           |           |           |           |           |           |           |           |
  +------------------------------------------------------------------------------------------------------------+
  |            |           |           |           |           |           |           |           |           |
  |    0014    | Nfrac<23> | Nfrac<22> | Nfrac<21> | Nfrac<20> | Nfrac<19> | Nfrac<18> | Nfrac<17> | Nfrac<16> |
  |            |           |           |           |           |           |           |           |           |
  +------------------------------------------------------------------------------------------------------------+
  |            |           |           |           |           |           |           |           |           |
  |    0015    | Nfrac<31> | Nfrac<30> | Nfrac<29> | Nfrac<28> | Nfrac<27> | Nfrac<26> | Nfrac<25> | Nfrac<24> |
  |            |           |           |           |           |           |           |           |           |
  +------------------------------------------------------------------------------------------------------------+
  |            |           |           |           |           |           |           |           |           |
  |    0016    |  Nmod<7>  |  Nmod<6>  |  Nmod<5>  |  Nmod<4>  |  Nmod<3>  |  Nmod<2>  |  Nmod<1>  |  Nmod<0>  |
  |            |           |           |           |           |           |           |           |           |
  +------------------------------------------------------------------------------------------------------------+
  |            |           |           |           |           |           |           |           |           |
  |    0017    |  Nmod<15> |  Nmod<14> |  Nmod<13> |  Nmod<12> |  Nmod<11> |  Nmod<10> |  Nmod<9>  |  Nmod<8>  |
  |            |           |           |           |           |           |           |           |           |
  +------------------------------------------------------------------------------------------------------------+
  |            |           |           |           |           |           |           |           |           |
  |    0018    |  Nmod<23> |  Nmod<22> |  Nmod<21> |  Nmod<20> |  Nmod<19> |  Nmod<18> |  Nmod<17> |  Nmod<16> |
  |            |           |           |           |           |           |           |           |           |
  +------------------------------------------------------------------------------------------------------------+
  |            |           |           |           |           |           |           |           |           |
  |    0019    |  Nmod<31> |  Nmod<30> |  Nmod<29> |  Nmod<28> |  Nmod<27> |  Nmod<26> |  Nmod<25> |  Nmod<24> |
  |            |           |           |           |           |           |           |           |           |
  +------------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+

*/


	// Integer Portion
	unsigned long Nint_top, Nint_bottom, Nint_bottom_word, Nint_top_word;

	SplitNumber_16Bit(Nint, &Nint_top, &Nint_bottom);

	Nint_bottom_word = Create24BitWord(Nint_bottom, REG_10h);
	Nint_top_word = Create24BitWord(Nint_top, REG_11h);

	sendWord_24Bit(Nint_bottom_word);
	sendWord_24Bit(Nint_top_word);

	// Fractional Portion
	unsigned long Nfrac_top, Nfrac_upper_middle, Nfrac_lower_middle, Nfrac_bottom;
	unsigned long Nfrac_top_word, Nfrac_uppermiddle_word, Nfrac_lowermiddle_word, Nfrac_bottom_word;

	SplitNumber_32Bit(Nfrac, &Nfrac_top, &Nfrac_upper_middle, &Nfrac_lower_middle, &Nfrac_bottom);

	Nfrac_bottom_word = Create24BitWord(Nfrac_bottom, REG_12h);
	Nfrac_lowermiddle_word = Create24BitWord(Nfrac_lower_middle, REG_13h);
	Nfrac_uppermiddle_word = Create24BitWord(Nfrac_upper_middle, REG_14h);
	Nfrac_top_word = Create24BitWord(Nfrac_top, REG_15h);

	sendWord_24Bit(Nfrac_bottom_word);
	sendWord_24Bit(Nfrac_lowermiddle_word);
	sendWord_24Bit(Nfrac_uppermiddle_word);
	sendWord_24Bit(Nfrac_top_word);

	// Modulus Portion
	unsigned long Nmod_top, Nmod_upper_middle, Nmod_lower_middle, Nmod_bottom;
	unsigned long Nmod_top_word, Nmod_uppermiddle_word, Nmod_lowermiddle_word, Nmod_bottom_word;

	SplitNumber_32Bit(Nmod, &Nmod_top, &Nmod_upper_middle, &Nmod_lower_middle, &Nmod_bottom);

	Nmod_bottom_word = Create24BitWord(Nmod_bottom, REG_16h);
	Nmod_lowermiddle_word = Create24BitWord(Nmod_lower_middle, REG_17h);
	Nmod_uppermiddle_word = Create24BitWord(Nmod_upper_middle, REG_18h);
	Nmod_top_word = Create24BitWord(Nmod_top, REG_19h);

	sendWord_24Bit(Nmod_bottom_word);
	sendWord_24Bit(Nmod_lowermiddle_word);
	sendWord_24Bit(Nmod_uppermiddle_word);
	sendWord_24Bit(Nmod_top_word);

}



/*******************************************************************************************************/
/*                                   Setup the phase adjustment                                        */
/*******************************************************************************************************/

void SetPhaseAdjustment(unsigned long PhaseAdjustment){

/*


 The 8V97003 offers the possibility to adjust the phase delay between the outputs (RF_OUT/ nRF_OUT) and the input (REF_IN)
 of the device by shifting the output phase by a fraction of the size of the fractional denominator, when the part is used
 in fractional mode. Writing to the Phase Adjustment Control Registers triggers a phase shift (refer to below table).
 The phase adjustment value set by the bits, Phase[31:0], should be less than the fractional-N denominator register, MOD.
 The actual phase shift can be obtained with the following equation:




                                      Phase     T_pfd
 Phase Adjustment (degrees) =  360 X ------- X -------
                                       MOD      T_out


           OR

                                   Phase
 Phase Adjustment (ns) =  T_pfd X -------
                                    MOD



 Where 360� represents one cycle of output clock, T_pfd is the period at the input to the PFD (in ns), T_out is the period at the
 output of the 8V97003, and Phase is a programmable value, the same bit length as MOD.





                                                                        Phase Adjustment Register Map

  +------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       ADDR       |        D7        |        D6        |        D5        |        D4        |        D3        |        D2        |        D1        |        D0        |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       001A       |     Phase<7>     |     Phase<6>     |    Phase<5>      |    Phase<4>      |    Phase<3>      |    Phase<2>      |    Phase<1>      |    Phase<0>      |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       001B       |    Phase<15>     |    Phase<14>     |    Phase<13>     |    Phase<12>     |    Phase<11>     |    Phase<10>     |     Phase<9>     |     Phase<8>     |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       001C       |    Phase<23>     |    Phase<22>     |    Phase<21>     |    Phase<20>     |    Phase<19>     |    Phase<18>     |    Phase<17>     |    Phase<16>     |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       001D       |    Phase<31>     |    Phase<30>     |    Phase<29>     |    Phase<28>     |    Phase<27>     |    Phase<26>     |    Phase<25>     |    Phase<24>     |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+


*/

	unsigned long top, upper_middle, lower_middle, bottom;
	unsigned long top_word, upper_middle_word, lower_middle_word, bottom_word;

	SplitNumber_32Bit(PhaseAdjustment, &top, &upper_middle, &lower_middle, &bottom);

	bottom_word = Create24BitWord(bottom, REG_1Ah);
	lower_middle_word = Create24BitWord(lower_middle, REG_1Bh);
	upper_middle_word = Create24BitWord(upper_middle, REG_1Ch);
	top_word = Create24BitWord(top, REG_1Dh);

	sendWord_24Bit(bottom_word);
	sendWord_24Bit(lower_middle_word);
	sendWord_24Bit(upper_middle_word);
	sendWord_24Bit(top_word);


}


/*******************************************************************************************************************/
/*                                         Setup the Delta Sigma Modulator                                         */
/*******************************************************************************************************************/

void controlDSM(bool DSMtype, bool ShapeDitherEn, bool DitherEn, unsigned long DSMorder, unsigned long DitherGain){

/*

                                                       DSM Control Register Map (Delta Sigma Modulator)
  +---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+
  |               |               |               |               |               |               |               |               |               |
  |     ADDR      |      D7       |      D6       |      D5       |      D4       |      D3       |      D2       |      D1       |      D0       |
  |               |               |               |               |               |               |               |               |               |
  +-----------------------------------------------------------------------------------------------------------------------------------------------+
  |               |               |               |               |               |               |               |               |               |
  |     001E      |    DSMType    |  DSMOrder<2>  |  DSMOrder<1>  |  DSMOrder<0>  |  DitherG<1>   |  DitherG<0>   | ShapeDitherEn |   DitherEn    |
  |               |               |               |               |               |               |               |               |               |
  +---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+


 In order to reduce the spurs, the user can enable the dither function to increase the repeat length of the code sequence in the Sigma Delta Modulator (DSM).
 The increased repeat length is 2^32 - 1 cycles so that the resulting quantization error is spread to appear like broadband noise. As a result, the
 in-band phase noise may be degraded when using the dither function. When the application requires the lowest possible phase noise and when the loop
 bandwidth is low enough to filter most of the undesirable spurs, or if the spurs won�t affect the system performance, it is recommended to use the low
 noise mode with dither disabled.



*/


	unsigned long DSMvalue, ShapeDitherEnValue, DitherEnValue, DSMshift, DitherGainshift, DSMresult, DSMword;

	if(DSMtype){ DSMvalue = BIT_7; }
	else if(!DSMtype){ DSMvalue = 0; }

	if(ShapeDitherEn){ ShapeDitherEnValue = BIT_1; }
	else if(!ShapeDitherEn){ ShapeDitherEnValue = 0; }

	if(DitherEn){ DitherEnValue = BIT_0; }
	else if(!DitherEn){ DitherEnValue = 0; }

	DSMshift = DSMorder << 4;

	DitherGainshift = DitherGain << 2;

	DSMresult = DSMvalue | ShapeDitherEnValue | DitherEnValue | DSMshift | DitherGainshift;

	DSMword = Create24BitWord(DSMresult, REG_1Eh);

	sendWord_24Bit(DSMword);

}



/*****************************************************************************************************************************/
/*                                            Setup the calibration control                                                  */
/*****************************************************************************************************************************/


void CalibrationControl(bool ForceRelock, bool PhaseAdjustTrigger, bool BandSelectDisable, unsigned long BandSelectAccuracy){


/*

                                                                     Calibration Control Register Map

  +------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       ADDR       |        D7        |        D6        |        D5        |        D4        |        D3        |        D2        |        D1        |        D0        |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       0021       |   ForceRelock    |   PhaseAdjTrig   |  BandSelDisable  |      UNUSED      |        0         |        0         |   BandSelAcc<1>  |   BandSelAcc<0>  |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+


____________________________________________________________________________

 ForceRelock

 0 = Normal operation (Default)
 1 = VCO forced to recalibrate.

 This bit is self-clearing.

____________________________________________________________________________

 Phase Adjust Trigger

 0 = Normal operation
 1 = Trigger phase adjustment once.

 This bit is self-clearing.

____________________________________________________________________________

 Band Select Disable

 This bit will prevent a VCO recalibration when registers 16:25 are written
   - Registers 16:25 are the feedback divider values Nint, Nfrac, & Nmod

 0 = VCO recalibrates when registers 16:25 are written
 1 = VCO does not recalibrate when registers 16:25 are written

____________________________________________________________________________

Band select/Calibration Accuracy

00 = 1x resolution
01 = 2x resolution
10 = 4x resolution
11 = 8x resolution (default)

____________________________________________________________________________



*/


	unsigned long FR_Part, PAT_Part, BSD_Part, Calibration, Calibration_word;

	// To reduce code complexity and size, i'm checking all three booleans individually and OR-ing them together at the end



	if(ForceRelock){ FR_Part = 0x80; } // Set the ForceRelock bit
	else if(!ForceRelock){ FR_Part = 0x00; }

	if(PhaseAdjustTrigger){PAT_Part = 0x40; } // Set the Phase Adjust Trigger bit
	else if(!PhaseAdjustTrigger){PAT_Part = 0x00; }

	if(BandSelectDisable){BSD_Part = 0x20; } // Set the Band Disable bit
	else if(!BandSelectDisable){ BSD_Part = 0x00; }

	Calibration = FR_Part | PAT_Part | BSD_Part | BandSelectAccuracy; // Put them all together to form the data byte

	Calibration_word = Create24BitWord(Calibration, REG_21h);

	sendWord_24Bit(Calibration_word);

}


/*******************************************************************************************************/
/*
 *                               Band Select Clock Divider Registers
 *
 *   Band Select Clock Divider:  Not Allowed: 0d
 *                               Default: 2,560d
 *                               Range: 1d - 131,071d
 *
 *   This ratio should be set so that F_pfd / BndSelDiv is < 100kHz.
 */
/*******************************************************************************************************/

void SetBandSelectClockDivider(unsigned short divider_value){


/*

                                                    Band Select Clock Diver Control Register Map

  +------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
  |      |                |                |                |                |                |                |                |                |
  | ADDR |       D7       |       D6       |       D5       |       D4       |       D3       |       D2       |       D1       |       D0       |
  |      |                |                |                |                |                |                |                |                |
  +----------------------------------------------------------------------------------------------------------------------------------------------+
  |      |                |                |                |                |                |                |                |                |
  | 0022 | BndSelDiv <07> | BndSelDiv <06> | BndSelDiv <05> | BndSelDiv <04> | BndSelDiv <03> | BndSelDiv <02> | BndSelDiv <01> | BndSelDiv <00> |
  |      |                |                |                |                |                |                |                |                |
  +----------------------------------------------------------------------------------------------------------------------------------------------+
  |      |                |                |                |                |                |                |                |                |
  | 0023 |     Unused     |     Unused     |     Unused     | BndSelDiv <12> | BndSelDiv <11> | BndSelDiv <10> | BndSelDiv <09> | BndSelDiv <08> |
  |      |                |                |                |                |                |                |                |                |
  +------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+



*/

	unsigned long upper, lower, upper_word, lower_word;

	SplitNumber_16Bit(divider_value, &upper, &lower);

	lower_word = Create24BitWord(lower, REG_22h);
	upper_word = Create24BitWord(upper, REG_23h);


	sendWord_24Bit(lower_word);
	sendWord_24Bit(upper_word);

}


/*****************************************************************************************************************************/
/*                                            Setup the lock detect registers                                                */
/*****************************************************************************************************************************/

void SetupLockDetect(bool Unlock_Detection, unsigned long LD_PinMode, unsigned long LD_Precision){

/*


                                                                        Lock Detect Control Register Map

  +------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       ADDR       |        D7        |        D6        |        D5        |        D4        |        D3        |        D2        |        D1        |        D0        |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       0026       |      UNUSED      |      UNUSED      |      UNUSED      |      UNUSED      | LDUnlockDetectEn |        0         |        0         |       0          |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       0027       |      UNUSED      |      UNUSED      |   LDPinMode<1>   |   LDPinMode<0>   |      UNUSED      |      LPD<2>      |      LPD<1>      |      LPD<0>      |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+


________________________________________________________________________

 Lock Detect - Unlock Detection

 0 = Disable (default)
 1 = Enable (a recalibration occurs if an unlock on LD is detected)

________________________________________________________________________

 LD Pin Mode

 00 = Digital Lock Detect (default); Normal lock detector function
 01 = Calibration done
 10 = Low
 11 = High

________________________________________________________________________

 Lock Detector Precision setting (ns)

 000 = 0.5 (default)
 001 = 1.0
 010 = 1.8
 011 = 3.0
 100 = 6.4
 101 = 6.4*
 110 = 10.4
 111 = 10.4*

 *The duplicate values are correct, according to the chip manufacturer.
          There are redundant settings for this parameter.
________________________________________________________________________


*/

	unsigned long unlock_detection_word, LD_PinMode_and_Precision, LD_PinMode_and_Precision_word;

	if(Unlock_Detection){

		unlock_detection_word = Create24BitWord(UNLOCK_DETECTION, REG_26h);
		sendWord_24Bit(unlock_detection_word);
	}

	else if(!Unlock_Detection){

		unlock_detection_word = Create24BitWord(NO_UNLOCK_DETECTION, REG_26h);
		sendWord_24Bit(unlock_detection_word);
	}

	LD_PinMode = LD_PinMode << 4;

	LD_PinMode_and_Precision = LD_PinMode | LD_Precision;

	LD_PinMode_and_Precision_word = Create24BitWord(LD_PinMode_and_Precision, REG_27h);

	sendWord_24Bit(LD_PinMode_and_Precision_word);


}


/***********************************************************************************************************************************************************************************/
/*                                                                    Setup the input control registers                                                                            */
/***********************************************************************************************************************************************************************************/

void SetupInputControlRegisters(bool inputType, bool refDoublerEn, bool multEnable, bool nMultReset, unsigned long refDivider, unsigned long multRatio, unsigned long nMultPwrDwn){


/*

                                                                Input Control Register Map

  +----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
  |                |                |                |                |                |                |                |                |                |
  |      ADDR      |       D7       |       D6       |      D5        |       D4       |       D3       |       D2       |       D1       |       D0       |
  |                |                |                |                |                |                |                |                |                |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                |                |                |                |                |                |                |                |                |
  |      0029      |     Ref<7>     |     Ref<6>     |     Ref<5>     |     Ref<4>     |     Ref<3>     |     Ref<2>     |     Ref<1>     |     Ref<0>     |
  |                |                |                |                |                |                |                |                |                |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                |                |                |                |                |                |                |                |                |
  |      002A      |     UNUSED     |     UNUSED     |     UNUSED     |     UNUSED     |   InputType    | RefDoubler_En  |     Ref<9>     |     Ref<8>     |
  |                |                |                |                |                |                |                |                |                |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                |                |                |                |                |                |                |                |                |
  |      002B      |    Mult_En     |  nMult_Reset   |     Mult<5>    |     Mult<4>    |     Mult<3>    |     Mult<2>    |     Mult<1>    |     Mult<0>    |
  |                |                |                |                |                |                |                |                |                |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                |                |                |                |                |                |                |                |                |
  |      002C      | nMultpwrdwn<2> | nMultpwrdwn<1> | nMultpwrdwn<0> |       0        |       0        |       1        |       0        |       0        |
  |                |                |                |                |                |                |                |                |                |
  +----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+


_____________________________________________________

 Ref[7:0]:

 Input reference divide value

 00 0000 0000 = Not Allowed
 00 0000 0001 = 1d (default)
 ...
 11 1111 1111 = 1023d

_____________________________________________________

 Input type:

 Selects either differential or single-ended input

 0 = Single-Ended Input
 1 = Differential Input (Default)

_____________________________________________________

 Reference Doubler Enable:

 Enables the Input Reference Doubler

 0 = Input reference doubler disabled
 1 = Input reference doubler enabled (default)

_____________________________________________________

 MULT Enable

 0 = MULT Not Enabled
 1 = MULT Enabled

_____________________________________________________

 nMult_reset:

 Resets the Reference multiplier block (MULT)

 0: Multiplier is reset (default)
 1: Multiplier is active

_____________________________________________________

 Mult[5:0]:

 Multiplication ratio for the multiplier block

 000000 = Unused
 000001 = Unused
 000010 = x2
 000011 = x3
 �
 111111 = x63

_____________________________________________________

 nMultpwrdwn[2:0]:

 MULT power down

 000: MULT Powered Down
 001: Not Allowed
 �
 110 = Not Allowed
 111 = MULT Powered Up

_____________________________________________________

*/

	unsigned long inputTypeValue, refDoublerEnValue, multEnableValue, nMultResetValue, nMultPwrDownshift, nMultPwrDownResult, nMultPwrDownWord;


	if(inputType){ inputTypeValue = BIT_3; }
	else if(!inputType){ inputTypeValue = 0; }

	if(refDoublerEn){ refDoublerEnValue = BIT_2; }
	else if(!refDoublerEn){ refDoublerEnValue = 0; }

	if(multEnable){ multEnableValue = BIT_7; }
	else if(!multEnable){ multEnableValue = 0; }

	if(nMultReset){ nMultResetValue = BIT_6; }
	else if(!nMultReset){ nMultResetValue = 0; }


	unsigned long refDividerBottom8Bits, refDividerTop2Bits, refDividerWord, refDividerResult, refDividerWord2;

	refDividerBottom8Bits = refDivider & BOTTOM_8_32Bit;

	refDividerTop2Bits = refDivider >> 8;

	refDividerWord = Create24BitWord(refDividerBottom8Bits, REG_29h);
	sendWord_24Bit(refDividerWord);

	refDividerResult = refDividerTop2Bits | inputTypeValue | refDoublerEnValue;

	refDividerWord2 = Create24BitWord(refDividerResult, REG_2Ah);
	sendWord_24Bit(refDividerWord2);

	unsigned long multResult, multResultWord;

	multResult = multEnableValue | nMultResetValue | multRatio;

	multResultWord = Create24BitWord(multResult, REG_2Bh);
	sendWord_24Bit(multResultWord);

	nMultPwrDownshift = nMultPwrDwn << 5;

	nMultPwrDownResult = nMultPwrDownshift | 0x04;

	nMultPwrDownWord = Create24BitWord(nMultPwrDownResult, REG_2Ch);
	sendWord_24Bit(nMultPwrDownWord);

}



/***************************************************************************************************************************************/
/*                                               Setup the charge pump registers                                                       */
/***************************************************************************************************************************************/

void SetupChargePump(unsigned long icp_pmos, unsigned long icp_nmos, bool CP_HiZ, unsigned long icp_bleeder){


/*

                                                             Charge Pump Control Register Map

  +----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
  |                |                |                |                |                |                |                |                |                |
  |      ADDR      |       D7       |       D6       |       D5       |       D4       |       D3       |       D2       |       D1       |       D0       |
  |                |                |                |                |                |                |                |                |                |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                |                |                |                |                |                |                |                |                |
  |      002D      |     UNUSED     |     UNUSED     |   Icp_pmos<5>  |   Icp_pmos<4>  |   Icp_pmos<3>  |   Icp_pmos<2>  |   Icp_pmos<1>  |   Icp_pmos<0>  |
  |                |                |                |                |                |                |                |                |                |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                |                |                |                |                |                |                |                |                |
  |      002E      |     UNUSED     |     UNUSED     |   Icp_nmos<5>  |   Icp_nmos<4>  |   Icp_nmos<3>  |   Icp_nmos<2>  |   Icp_nmos<1>  |   Icp_nmos<0>  |
  |                |                |                |                |                |                |                |                |                |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                |                |                |                |                |                |                |                |                |
  |      002F      |     UNUSED     |     UNUSED     |     CP_HiZ     | Icp_bleeder<4> | Icp_bleeder<3> | Icp_bleeder<2> | Icp_bleeder<1> | Icp_bleeder<0> |
  |                |                |                |                |                |                |                |                |                |
  +----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+


 In order to put the charge pump into three-state mode, the user must set the bits CP_HiZ (Bit D5) to 1 in Register 47d in the Charge Pump Control Registers.

 This bit should be set to 0 for normal operation.

________________________________________________

 Charge Pump Pmos Current Setting:

 00 0000 = 166 uA

 00 0001 = 333 uA

 ...

 01 1101 = 5 mA (default)

 ...

 11 1111 = 10.66 mA

________________________________________________

 Charge Pump Nmos Current Setting

 00 0000 = 166 uA

 00 0001 = 333 uA

 ...

 01 1101 = 5 mA (default)

 ...

 01 1111 = 10.66 mA

________________________________________________

 Charge Pump High-Impedance Control

 0 = Charge Pump Active

 1 = Charge Pump High-Impedance



 Charge Pump High Impedance Macros:

 CP_HIGH_Z 0x00000020

________________________________________________

 Charge Pump Bleeder Current Setting

 0 0000 = Off (0 uA) (Default)

 0 0001 = 5.3 uA

 0 0010 = 10.6 uA

 ...

 1 1111 = 166 uA

________________________________________________

*/


	unsigned long icp_pmos_word, icp_nmos_word, icp_bleeder_word, bleeder_with_HiZ;

	icp_pmos_word = Create24BitWord(icp_pmos, REG_2Dh);
	icp_nmos_word = Create24BitWord(icp_nmos, REG_2Eh);


	if(CP_HiZ){

		bleeder_with_HiZ = icp_bleeder | CP_HIGH_Z;

		icp_bleeder_word = Create24BitWord(bleeder_with_HiZ, REG_2Fh);
	}

	else{

		icp_bleeder_word = Create24BitWord(icp_bleeder, REG_2Fh);
	}


	sendWord_24Bit(icp_pmos_word);
	sendWord_24Bit(icp_nmos_word);
	sendWord_24Bit(icp_bleeder_word);

}


/***************************************************************************************************************************************/
/*                                         Setup the VCO resolution control registers                                                  */
/***************************************************************************************************************************************/

void setVcoResolution(unsigned long bias_value){

/*

                                         VCO Resolution Control Register Map
  +------+------------+------------+------------+------------+------------+------------+------------+------------+
  |      |            |            |            |            |            |            |            |            |
  | ADDR |     D7     |     D6     |     D5     |     D4     |     D3     |     D2     |     D1     |     D0     |
  |      |            |            |            |            |            |            |            |            |
  +--------------------------------------------------------------------------------------------------------------+
  |      |            |            |            |            |            |            |            |            |
  | 0030 | vco_dac<7> | vco_dac<6> | vco_dac<5> | vco_dac<4> | vco_dac<3> | vco_dac<2> | vco_dac<1> | vco_dac<0> |
  |      |            |            |            |            |            |            |            |            |
  +------+------------+------------+------------+------------+------------+------------+------------+------------+


 VCO Bias DAC Setting (Resolution = 6.18mV / step)

 0000 0000 = 489 mV
 0000 0001 = 495.2 mV
 0000 0010 = 501.4 mV
 �
 0110 0010 = 1.095 V (default)
 �
 1111 1111 = 2.067 V

*/


	unsigned long VcoResolutionWord;

	VcoResolutionWord = Create24BitWord(bias_value, REG_30h);

	sendWord_24Bit(VcoResolutionWord);

}



/***********************************************************************************************************************************************************************************/
/*                                                                   Setup the output control registers                                                                            */
/***********************************************************************************************************************************************************************************/

void SetupOutputControl(bool QA_enable, unsigned long QA_power, bool QB_enable, unsigned long QB_power, bool MuteUntil_LockDetect, bool OutDoublerEnable, unsigned long OutDivider){

/*

  The 8V97003 device provides two differential outputs. These two outputs generate the same frequency equal to f_VCO when bypassing the optional output doubler
  and the optional output divider M0, or to 2 � f_VCO (up to 18GHz) when using the optional output doubler, or an integer division of the VCO frequency f_VCO.
  The division ratios of the output divider are provided in the Outputs Control Registers (see below).

  Each output buffer RF_OUTA and RF_OUTB offer a programmable RF output power. The programmable output power settings can be selected from -2dBm to +12dBm with steps of 2dBm.
  The RF output power can be programmed via the bits QA_pwr[7:0] and QB_pwr[7:0].

  The 8V97003 offers an auxiliary output (RF_OUTB). If the auxiliary output stage is not used, it can be powered down by using the QB_Ena bit in the Outputs Control Registers
  (refer to below register map). The outputs can be disabled until the part achieves lock. To enable this mode, the user will set the Mute_until_LD bit in the Outputs Control
  Registers (refer to below register map). The MUTE pin can be used to mute all outputs and be used as a similar function.


                                                               Output Control Register Map

  +---------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
  |               |                |                |                |                |                |                |                |                |
  |     ADDR      |       D7       |       D6       |       D5       |       D4       |       D3       |       D2       |       D1       |       D0       |
  |               |                |                |                |                |                |                |                |                |
  +-------------------------------------------------------------------------------------------------------------------------------------------------------+
  |               |                |                |                |                |                |                |                |                |
  |     0033      |    QA_pwr<7>   |    QA_pwr<6>   |    QA_pwr<5>   |    QA_pwr<4>   |    QA_pwr<3>   |    QA_pwr<2>   |    QA_pwr<1>   |    QA_pwr<0>   |
  |               |                |                |                |                |                |                |                |                |
  +-------------------------------------------------------------------------------------------------------------------------------------------------------+
  |               |                |                |                |                |                |                |                |                |
  |     0034      |       0        |     UNUSED     | Mute_Until_LD  |     QA_ena     |       1        |       0        |       0        |       0        |
  |               |                |                |                |                |                |                |                |                |
  +-------------------------------------------------------------------------------------------------------------------------------------------------------+
  |               |                |                |                |                |                |                |                |                |
  |     0035      |   QB_pwr<7>    |    QB_pwr<6>   |    QB_pwr<5>   |    QB_pwr<4>   |    QB_pwr<3>   |    QB_pwr<2>   |    QB_pwr<1>   |    QB_pwr<0>   |
  |               |                |                |                |                |                |                |                |                |
  +-------------------------------------------------------------------------------------------------------------------------------------------------------+
  |               |                |                |                |                |                |                |                |                |
  |     0036      |       0        |     UNUSED     |     UNUSED     |     QB_ena     |       1        |       0        |       0        |       0        |
  |               |                |                |                |                |                |                |                |                |
  +-------------------------------------------------------------------------------------------------------------------------------------------------------+
  |               |                |                |                |                |                |                |                |                |
  |     0037      |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |
  |               |                |                |                |                |                |                |                |                |
  +-------------------------------------------------------------------------------------------------------------------------------------------------------+
  |               |                |                |                |                |                |                |                |                |
  |     0038      |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |
  |               |                |                |                |                |                |                |                |                |
  +-------------------------------------------------------------------------------------------------------------------------------------------------------+
  |               |                |                |                |                |                |                |                |                |
  |     0039      |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |
  |               |                |                |                |                |                |                |                |                |
  +-------------------------------------------------------------------------------------------------------------------------------------------------------+
  |               |                |                |                |                |                |                |                |                |
  |     003A      |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |
  |               |                |                |                |                |                |                |                |                |
  +-------------------------------------------------------------------------------------------------------------------------------------------------------+
  |               |                |                |                |                |                |                |                |                |
  |     003B      | OutDoubler_Ena |     UNUSED     |     UNUSED     |     UNUSED     |     UNUSED     |   OutDiv <2>   |   OutDiv <1>   |   OutDiv <0>   |
  |               |                |                |                |                |                |                |                |                |
  +---------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+


__________________________________________________________________________________

 QA_Power:

 RF_OUTA Power Setting

 0000 0000 = OFF (Default)
 0000 0001 = -2dBm
 0000 0011 = 0dBm
 0000 0111 = +2dBm
 0000 1111 = +4dBm
 0001 1111 = +6dBm
 0011 1111 = +8dBm
 0111 1111 = +10dBm
 1111 1111 = +12dBm

__________________________________________________________________________________

 QA_ena:

 RF_OUTA Enable

 0 = RF_OUTA is OFF (Default)
 1 = RF_OUTA is Enabled

__________________________________________________________________________________

 QB_Power:

 RF_OUTB Power Setting

 0000 0000 = OFF (Default)
 0000 0001 = -2dBm
 0000 0011 = 0dBm
 0000 0111 = +2dBm
 0000 1111 = +4dBm
 0001 1111 = +6dBm
 0011 1111 = +8dBm
 0111 1111 = +10dBm
 1111 1111 = +12dBm

__________________________________________________________________________________

 QB_ena:

 RF_OUTB Enable

 0 = RF_OUTB is OFF (Default)
 1 = RF_OUTB is Enabled

__________________________________________________________________________________

 Mute_Until_LD:

 Mute until Lock Detect selection

 0: Outputs are enabled independent of Lock Detect (Default)
 1: Outputs are enabled only when Lock Detect is high

__________________________________________________________________________________

 Out_Doubler_Ena:

 RF Output Doubler Enable

 0 = RF Output Doubler Disabled (default)
 1 = RF Output Doubler Enabled

 OutDoubler_Ena may only be set to 1 if the VCO frequency is not greater than 9GHz.

__________________________________________________________________________________

 OutDivider:

 RF Output Divider Settings

 000 = Divide By 1
 001 = Divide By 2
 010 = Divide By 4
 011 = Divide By 8
 100 = Divide By 16
 101 = Divide By 32
 110 = Unused
 111 = Unused

__________________________________________________________________________________


*/


	if(QA_enable){

		unsigned long QA_power_word, QA_enable_word;

		QA_power_word = Create24BitWord(QA_power, REG_33h);

		sendWord_24Bit(QA_power_word);

		if(MuteUntil_LockDetect){ QA_enable_word = Create24BitWord(QA_ENABLE_MUTE_UNTIL_LOCK_DETECT, REG_34h); }

		else{ QA_enable_word = Create24BitWord(QA_ENABLE_NO_MUTE, REG_34h); }

		sendWord_24Bit(QA_enable_word);

	}

	if(QB_enable){

		unsigned long QB_power_word, QB_enable_word, Enable_MuteUntilLockDetect_WithoutQA_word, Disable_MuteUntilLockDetect_WithoutQA_word;

		QB_power_word = Create24BitWord(QB_power, REG_35h);

		sendWord_24Bit(QB_power_word);

		if(QA_enable){

			// QA already set the MuteUntilLockDetect bit to the appropriate value, so we only need to enable the QB_enable bit
			QB_enable_word = Create24BitWord(QB_ENABLE, REG_36h);
			sendWord_24Bit(QB_enable_word);

		}

		else if(MuteUntil_LockDetect){

			// Set the MuteUntilLockDetect Bit inside register 34h since QA didn't set it already
			Enable_MuteUntilLockDetect_WithoutQA_word = Create24BitWord(MUTE_UNTIL_LOCK_DETECT_WITHOUT_QA_ENABLED, REG_34h);
			sendWord_24Bit(Enable_MuteUntilLockDetect_WithoutQA_word);

			// Set the QB_enable pin
			QB_enable_word = Create24BitWord(QB_ENABLE, REG_36h);
			sendWord_24Bit(QB_enable_word);

		}

		else if(!MuteUntil_LockDetect){

			// Clear the MuteUntilLockDetect Bit inside register 34h since QA didn't do it already
			Disable_MuteUntilLockDetect_WithoutQA_word = Create24BitWord(DISABLE_MUTE_WITHOUT_QA_ENABLED, REG_34h);
			sendWord_24Bit(Disable_MuteUntilLockDetect_WithoutQA_word);

			// Set the QB_enable pin
			QB_enable_word = Create24BitWord(QB_ENABLE, REG_36h);
			sendWord_24Bit(QB_enable_word);

		}

	}

	unsigned long output_divider_word, output_divider_value;

	if(OutDoublerEnable){

		output_divider_value = OutDivider | OutputDoublerEnable;
		output_divider_word = Create24BitWord(output_divider_value, REG_3Bh);
		sendWord_24Bit(output_divider_word);
	}

	else{
		output_divider_word = Create24BitWord(OutDivider, REG_3Bh);
		sendWord_24Bit(output_divider_word);
	}

}


/***********************************************************************************************************************************************************************************/
/*                                                          Setup the additional calibration control register                                                                      */
/***********************************************************************************************************************************************************************************/

void SetCalibrationVoltage(unsigned long CaliVoltage){

/*


                                                                    Additional Calibration Control Register Map

  +------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       ADDR       |        D7        |        D6        |        D5        |        D4        |        D3        |        D2        |        D1        |        D0        |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       003C       |      UNUSED      |      UNUSED      |      UNUSED      |      UNUSED      |  CaliVoltage<3>  |  CaliVoltage<2>  |  CaliVoltage<1>  |  CaliVoltage<0>  |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+


____________________________________________________________________

 Calibration Voltage Setting, sets Vc voltage during calibration

 0000 = 0.29V (default)
 0001 = 0.56V
 0010 = 0.83V
 0011 = 1.1V
 0100 = 1.37V
 0101 = 1.645V
 0110 = 1.915V
 0111 = 2.185V
 1000 = 2.455V
 1001 = 2.725V
 1010 = 3.0V

 1011 - 1111 = Unused

____________________________________________________________________


*/

	unsigned long CaliVoltage_word;

	CaliVoltage_word = Create24BitWord(CaliVoltage, REG_3Ch);

	sendWord_24Bit(CaliVoltage_word);

}