#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_


#include "GlobalStandards.h"


#define CHANNELS_OFF 0x00 	//!< channel state when all systems are off
#define CHNNELS_ON	 0XFF	//!< channel
#define SYSTEM0		 0x01	//!< channel state when 'SYSTEM0' is on
#define SYSTEM1		 0x02	//!< channel state when 'SYSTEM1' is on
#define SYSTEM2 	 0x04	//!< channel state when 'SYSTEM2' is on
#define SYSTEM3		 0x08	//!< channel state when 'SYSTEM3' is on
#define SYSTEM4		 0x10	//!< channel state when 'SYSTEM4' is on
#define SYSTEM5		 0x20	//!< channel state when 'SYSTEM5' is on
#define SYSTEM6 	 0x40	//!< channel state when 'SYSTEM6' is on
#define SYSTEM7 	 0x80	//!< channel state when 'SYSTEM7' is on


#define NUMBER_OF_THRESHOLD_VOLTAGES 	6 		///< first 3 are charging voltages, last 3 are discharging voltages
#define DEFAULT_EPS_THRESHOLD_VOLTAGES 	{(voltage_t)6500, (voltage_t)7100, (voltage_t)7300,	(voltage_t)6600, (voltage_t)7200, (voltage_t)7400}

typedef enum{
    Startup,
	FullMode,
	CruiseMode,
	SafeMode,
	CriticalMode
}State_t;

typedef enum __attribute__ ((__packed__)){
    INDEX_DOWN_SAFE,
    INDEX_DOWN_CRUISE,
    INDEX_DOWN_FULL,
    INDEX_UP_SAFE,
    INDEX_UP_CRUISE,
    INDEX_UP_FULL
}EpsThresholdsIndex;

typedef union __attribute__ ((__packed__)){
    voltage_t raw[NUMBER_OF_THRESHOLD_VOLTAGES];
    struct {
        voltage_t Vdown_safe;
        voltage_t Vdown_cruise;
        voltage_t Vdown_full;
        voltage_t Vup_safe;
        voltage_t Vup_cruise;
        voltage_t Vup_full;
    }fields;
}EpsThreshVolt_t;

/*!
 * @brief initializes the state machine.
 * @return	0 on success
 * 			-1 on failure of init
 */
int StateMachine_init();

/*!
 * @brief Executes the necessary procedure in order to initiate the system into Full mode
 * @return	0 on success
 * 			errors according to <hal/errors.h>
 */
int EnterFullMode();

/*!
 * @brief Executes the necessary procedure in order to initiate the system into Cruise mode
 * @return	0 on success
 * 			errors according to <hal/errors.h>
 */
int EnterCruiseMode();

/*!
 * @brief Executes the necessary procedure in order to initiate the system into Safe mode
 * @return	0 on success
 * 			errors according to <hal/errors.h>
 */
int EnterSafeMode();

/*!
 * @brief Executes the necessary procedure in order to initiate the system into Critical mode
 * @return	0 on success
 * 			errors according to <hal/errors.h>
 */
int EnterCriticalMode();

/*!
 * @brief Sets the channel state according to the bitwise 'logic on'
 * if the 2'nd bit is '1' the second channel will turn on (channel = 0b01000000)
 * @note please use the defines defined in this header to turn on/off channels
 * @return	0 on success
 * 			errors according to <hal/errors.h>
 */
int SetEPS_Channels(channel_t channel);

/*!
 * returns the current system state according to the EpsState_t enumeration
 * @return system state according to EpsState_t
 */
State_t GetSystemState();

/*
 * Gets the current system channel state
 * @return current system channel state
 */
channel_t GetSystemChannelState();

Boolean EpsGetLowVoltageFlag();

void EpsSetLowVoltageFlag(Boolean low_volt_flag);

/*!
 * @brief setting the new EPS logic threshold voltages on the FRAM.
 * @param[in] thresh_volts an array holding the new threshold values
 * @return	0 on success
 * 			-1 on failure setting new threshold voltages
 * 			-2 on invalid thresholds
 * 			ERR according to <hal/errors.h>
 */
int UpdateThresholdVoltages(EpsThreshVolt_t *thresh_volts);

/*!
 * @brief getting the EPS logic threshold  voltages on the FRAM.
 * @param[out] thresh_volts a buffer to hold the threshold values
 * @return	0 on success
 * 			-1 on NULL input array
 * 			-2 on FRAM read errors
 */
int GetThresholdVoltages(EpsThreshVolt_t thresh_volts[NUMBER_OF_THRESHOLD_VOLTAGES]);


/*!
 * @brief	setting the new EPS logic threshold voltages on the FRAM to the default.
 * @return	0 on success
 * 			-1 on failure setting smoothing factor
  * @see EPS_DEFAULT_THRESHOLD_VOLTAGES
 */
int RestoreDefaultThresholdVoltages();

/*
 * @brief change the state machine's state based on filtered voltage from the EPS
 * @param voltage input voltage from EPS
 * @return  0 on success
 *          errors according to <hal/errors.h>
 */
int ChangeStateByVoltage(voltage_t voltage);

#endif /* STATEMACHINE_H_ */
