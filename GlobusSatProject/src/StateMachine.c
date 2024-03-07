#include "StateMachine.h"
#include <stdio.h>
#include <string.h>

/* global variables */
State_t state;
EpsThreshVolt_t curr_thresh_volts[NUMBER_OF_THRESHOLD_VOLTAGES];

int StateMachine_init() {
    state = Startup;

    FRAM_read((unsigned char*) &curr_thresh_volts, EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE);

}

int EnterFullMode();

int EnterCruiseMode();

int EnterSafeMode();

int EnterCriticalMode();

int SetEPS_Channels(channel_t channel);

State_t GetSystemState() {
    return state;
}

channel_t GetSystemChannelState();

Boolean EpsGetLowVoltageFlag();

void EpsSetLowVoltageFlag(Boolean low_volt_flag);

int UpdateThresholdVoltages(EpsThreshVolt_t *thresh_volts) {
    memcpy(curr_thresh_volts, thresh_volts[NUMBER_OF_THRESHOLD_VOLTAGES], sizeof(curr_thresh_volts));
    FRAM_write((unsigned char*) &curr_thresh_volts, EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE);
}

int GetThresholdVoltages(EpsThreshVolt_t thresh_volts[NUMBER_OF_THRESHOLD_VOLTAGES]) {
    memcpy(thresh_volts[NUMBER_OF_THRESHOLD_VOLTAGES], curr_thresh_volts, sizeof(curr_thresh_volts));
}

int ChangeStateByVoltage(voltage_t voltage) {
    switch (GetSystemState()) {
        case CriticalMode:
            if(voltage > curr_thresh_volts.fields.Vup_safe) {
                EnterSafeMode();
            }
            break;
        case SafeMode:
            if(voltage < curr_thresh_volts.fields.Vdown_safe) {
                EnterCriticalMode();
            }
            if(voltage > curr_thresh_volts.fields.Vup_cruise) {
                EnterCruiseMode();
            }
            break;
        case CruiseMode:
            if(voltage < curr_thresh_volts.fields.Vdown_cruise) {
                EnterSafeMode();
            }
            if(voltage > curr_thresh_volts.fields.Vup_full) {
                EnterFullMode();
            }
            break;
        case FullMode:
            if(voltage < curr_thresh_volts.fields.Vdown_full) {
                EnterCruiseMode();
            }
            break;
        default:
            break;
    }
}

