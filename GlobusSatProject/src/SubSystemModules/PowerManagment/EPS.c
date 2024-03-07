/*
 * @file	EPS.c
 * @brief	EPS- Energy Powering System.This system is incharge of the energy consumtion
 * 			the satellite and switching on and off power switches(5V, 3V3)
 * @see		inspect logic flowcharts thoroughly in order to write the code in a clean manner.
 */
#include "EPS.h"
#include "../../utils.h"
#include "../../StateMachine.h"

/* Global variables */
float curr_alpha;
voltage_t filtered_voltage;



/*!
 * @brief initializes the EPS subsystem.
 * @return	0 on success
 * 			-1 on failure of init
 */
int EPS_Init() {
    // initialize drivers and solar panel

    FRAM_read((unsigned char*) &curr_alpha, EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE);
    filtered_voltage = 0;

    EPS_Conditioning();

}

int update_filtered_voltage() {
    voltage_t vbat = 0;
    if (GetBatteryVoltage(&vbat) != 0) {
        return -1;
    }
    filtered_voltage = curr_alpha * vbat + (1-curr_alpha) * filtered_voltage;
    return 0;
}

/*!
 * @brief EPS logic. controls the state machine of which subsystem
 * is on or off, as a function of only the battery voltage
 * @return	0 on success
 * 			-1 on failure setting state of channels
 */
int EPS_Conditioning() {
    if (update_filtered_voltage() != 0) {
        return -1;
    }

}

/*!
 * @brief returns the current voltage on the battery
 * @param[out] vbat he current battery voltage
 * @return	0 on success
 * 			Error code according to <hal/errors.h>
 */
int GetBatteryVoltage(voltage_t *vbat);

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
 * @brief getting the smoothing factor (alpha) from the FRAM.
 * @param[out] alpha a buffer to hold the smoothing factor
 * @return	0 on success
 * 			-1 on NULL input array
 * 			-2 on FRAM read errors
 */
int GetAlpha(float *alpha);

/*!
 * @brief setting the new voltage smoothing factor (alpha) on the FRAM.
 * @param[in] new_alpha new value for the smoothing factor alpha
 * @note new_alpha is a value in the range - (0,1)
 * @return	0 on success
 * 			-1 on failure setting new smoothing factor
 * 			-2 on invalid alpha
 * @see LPF- Low Pass Filter at wikipedia: https://en.wikipedia.org/wiki/Low-pass_filter#Discrete-time_realization
 */
int UpdateAlpha(sat_packet_t *cmd);

/*!
 * @brief setting the new voltage smoothing factor (alpha) to be the default value.
 * @return	0 on success
 * 			-1 on failure setting new smoothing factor
 * @see DEFAULT_ALPHA_VALUE
 */
int RestoreDefaultAlpha();

/*!
 * @brief	setting the new EPS logic threshold voltages on the FRAM to the default.
 * @return	0 on success
 * 			-1 on failure setting smoothing factor
  * @see EPS_DEFAULT_THRESHOLD_VOLTAGES
 */
int RestoreDefaultThresholdVoltages();

int CMDGetHeaterValues(sat_packet_t *cmd);

int CMDSetHeaterValues(sat_packet_t *cmd);

