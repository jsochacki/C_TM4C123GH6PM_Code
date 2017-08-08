/*
 * Synth_API_Control_Registers.h
 *
 * Author: Mason Edgar
 *
 */

#ifndef SYNTH_API_CONTROL_REGISTERS_H_
#define SYNTH_API_CONTROL_REGISTERS_H_

extern void SetFeedbackControlValues(unsigned short Nint, unsigned long Nfrac, unsigned long Nmod);
extern void SetPhaseAdjustment(unsigned long PhaseAdjustment);
extern void controlDSM(bool DSMtype, bool ShapeDitherEn, bool DitherEn, unsigned long DSMorder, unsigned long DitherGain);
extern void CalibrationControl(bool ForceRelock, bool PhaseAdjustTrigger, bool BandSelectDisable, unsigned long BandSelectAccuracy);
extern void SetBandSelectClockDivider(unsigned short divider_value);
extern void SetupLockDetect(bool Unlock_Detection, unsigned long LD_PinMode, unsigned long LD_Precision);
extern void SetupInputControlRegisters(bool inputType, bool refDoublerEn, bool multEnable, bool nMultReset, unsigned long refDivider, unsigned long multRatio, unsigned long nMultPwrDwn);
extern void SetupChargePump(unsigned long icp_pmos, unsigned long icp_nmos, bool CP_HiZ, unsigned long icp_bleeder);
extern void setVcoResolution(unsigned long bias_value);
extern void SetupOutputControl(bool QA_enable, unsigned long QA_power, bool QB_enable, unsigned long QB_power, bool MuteUntil_LockDetect, bool OutDoublerEnable, unsigned long OutDivider);
extern void SetCalibrationVoltage(unsigned long CaliVoltage);


#endif /* SYNTH_API_CONTROL_REGISTERS_H_ */
