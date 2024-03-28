/*
 * @file	EPS.c
 * @brief	EPS- Energy Powering System.This system is incharge of the energy consumtion
 * 			the satellite and switching on and off power switches(5V, 3V3)
 * @see		inspect logic flowcharts thoroughly in order to write the code in a clean manner.
 */
#include "EPS.h"
#include "utils.h"
#include "StateMachine.h"

#ifdef TESTING
#include "EpsStub.h"
#define imepsv2_piu__gethousekeepingeng imepsv2_piu__gethousekeepingeng_stub
#else
#include <satellite-subsystems/imepsv2_piu.h>
#endif

/* Global variables */
voltage_t filtered_voltage;


int EPS_Init() {
    // initialize drivers and solar panel

    filtered_voltage = 0;

    return EPS_Conditioning();

}

int update_filtered_voltage() {
    voltage_t vbat = 0;
    if (GetBatteryVoltage(&vbat) != 0) {
        return -1;
    }
    filtered_voltage = curr_alpha * vbat + (1-curr_alpha) * filtered_voltage;
    return 0;
}

int EPS_Conditioning() {
    if (update_filtered_voltage() != 0) {
        return -1;
    }

    return ChangeStateByVoltage(filtered_voltage):
}

int GetBatteryVoltage(voltage_t *vbat) {

    imepsv2_piu__gethousekeepingeng__from_t response;


    int error = imepsv2_piu__gethousekeepingeng(0,&response);
    if (error) {
        return error;
    }

    *vbat = response.fields.batt_input.fields.volt;
    return 0;
}

int GetAlpha(float *alpha) {
    if (alpha == NULL) {
        return -1;
    }

    int err = FRAM_read((unsigned char*) &alpha, EPS_ALPHA_FILTER_VALUE_ADDR, EPS_ALPHA_FILTER_VALUE_SIZE);
    if (err != 0) {
        return -1;
    }
    return 0;
}

int UpdateAlpha(float new_alpha) {
    if (new_alpha <= 0 || new_alpha > 1) {
        return -1;
    }

    int err = FRAM_write((unsigned char*) &new_alpha, EPS_ALPHA_FILTER_VALUE_ADDR, EPS_ALPHA_FILTER_VALUE_SIZE);

    if (err != 0) {
        return -2;
    }
    return 0;
}

/*!
 * @brief setting the new voltage smoothing factor (alpha) to be the default value.
 * @return	0 on success
 * 			-1 on failure setting new smoothing factor
 * @see DEFAULT_ALPHA_VALUE
 */
int RestoreDefaultAlpha() {
    return UpdateAlpha(DEFAULT_ALPHA_VALUE);
}

int CMDGetHeaterValues(sat_packet_t *cmd);

int CMDSetHeaterValues(sat_packet_t *cmd);

