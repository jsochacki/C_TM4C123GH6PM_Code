/*
 * Synth_API_SPI_Setup.h
 *
 * Author: Mason Edgar
 */

#ifndef SYNTH_API_SPI_SETUP_H_
#define SYNTH_API_SPI_SETUP_H_


extern void setupClock(void);
extern void setupChipSelect(int mode);
extern void setupSPI_8Bit(void);
extern void SplitNumber_16Bit(unsigned short original, unsigned long* top_half, unsigned long* bottom_half);
extern void SplitNumber_32Bit(unsigned long original, unsigned long* top_portion, unsigned long* upper_middle_portion, unsigned long* lower_middle_portion, unsigned long* bottom_portion);
extern void FixedPointMaximizeMOD(double ratio, unsigned long* nFRAC, unsigned long* nMOD);
extern void FixedPointMinimizeMOD(double ratio, unsigned long* nFRAC, unsigned long* nMOD);
extern void DetermineFeedbackValues(double freq_ratio, unsigned short* nINT, unsigned long* nFRAC, unsigned long* nMOD);
extern unsigned long Create24BitWord(unsigned long data, unsigned long reg);
extern void sendWord_24Bit(unsigned long word);
extern uint32_t GetRegisterValue(unsigned long address);
extern uint32_t readWord_24Bit(unsigned long word);



#endif /* SYNTH_API_SPI_SETUP_H_ */
