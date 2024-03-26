#include "StateMachine.h"
#include <stdio.h>
#include <string.h>

/* global variables */
State_t state;

int StateMachine_init() {
    state = Startup;

    return 0;
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
    int error = FRAM_write((unsigned char*) &thresh_volts, EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE);
    if (error != 0) {
        return -1;
    }
    return 0;
}

int GetThresholdVoltages(EpsThreshVolt_t thresh_volts[NUMBER_OF_THRESHOLD_VOLTAGES]) {
    int error = FRAM_read((unsigned char*) &thresh_volts, EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE);
    if (error != 0) {
        return -1;
    }
    return 0;
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

