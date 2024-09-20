#include "StateMachine.h"
#include <stdio.h>
#include <string.h>

/* global variables */
State_t state;

int StateMachine_init() {
    state = Startup;

    return 0;
}

int EnterFullMode() {
    state = FullMode;
    return 0;
}

int EnterCruiseMode() {
    state = CruiseMode;
    return 0;
}

int EnterSafeMode() {
    state = SafeMode;
    return 0;
}

int EnterCriticalMode() {
    state = CriticalMode;
    return 0;
}

//int SetEPS_Channels(channel_t channel);

State_t GetSystemState() {
    return state;
}

//channel_t GetSystemChannelState();

Boolean TransmissionAllowedByState() {
	return (state == FullMode || state == CruiseMode) ? TRUE : FALSE;
}

int UpdateThresholdVoltages(EpsThreshVolt_t *thresh_volts) {
    int error = FRAM_write((unsigned char*) thresh_volts, EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE);
    if (error != 0) {
    	logError(state_machine, __LINE__, error, "FRAM write failed");
        return error;
    }
    return 0;
}

int RestoreDefaultThresholdVoltages() {
    EpsThreshVolt_t def = DEFAULT_EPS_THRESHOLD_VOLTAGES;
    return UpdateThresholdVoltages(&def);
}

int GetThresholdVoltages(EpsThreshVolt_t thresh_volts[NUMBER_OF_THRESHOLD_VOLTAGES]) {
    int error = FRAM_read((unsigned char*) thresh_volts, EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE);
    if (error != 0) {
    	logError(state_machine, __LINE__, error, "FRAM read failed");
        return error;
    }
    return 0;
}

int ChangeStateByVoltage(voltage_t voltage) {
	EpsThreshVolt_t curr_thresh_volts;
	int err = GetThresholdVoltages(&curr_thresh_volts);
	if (err != 0) {
		logError(state_machine, __LINE__, err, "get threshold voltages failed");
		return err;
	}

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
    return 0;
}

