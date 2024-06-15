#include "EpsTestingDemo.h"
#include "EpsStub.h"
#include "SubSystemModules/PowerManagement/EPS.h"
#include "StateMachine.h"
#include "math.h"
#include <stdio.h>

#define EPSILON 1e-6

Boolean TestAlphaChanges() {
    printf("Getting alpha value \n");
    float initial_alpha;
    if (GetAlpha(&initial_alpha) != 0) {
        printf("Failed to get alpha value \n");
        return FALSE;
    } else {
        printf("Initial alpha: %f", initial_alpha);
    }

    printf("Updating alpha value \n");
    if (UpdateAlpha(0.1) != 0) {
        printf("Failed to update alpha value \n");
        return FALSE;
    } else {
        printf("Updated alpha to 0.1");
    }

    printf("Getting updated alpha value \n");
    float updated_alpha;
    if (GetAlpha(&updated_alpha) != 0) {
        printf("Failed to get alpha value \n");
        return FALSE;
    } else {
        printf("Updated alpha: %f", updated_alpha);
        if (fabs(updated_alpha - 0.1) > EPSILON) {
            printf("Updated alpha is incorrect\n");
            return FALSE;
        }
    }

    printf("Restoring default alpha value \n");
    if (RestoreDefaultAlpha() != 0) {
        printf("Failed to restore default alpha value \n");
        return FALSE;
    } else {
        printf("Restored default value alpha\n");
    }

    printf("Getting updated alpha value \n");
    if (GetAlpha(&updated_alpha) != 0) {
        printf("Failed to get alpha value \n");
        return FALSE;
    } else {
        printf("Updated alpha: %f", updated_alpha);
        if (fabs(updated_alpha - DEFAULT_ALPHA_VALUE) > EPSILON) {
            printf("Updated alpha is incorrect\n");
            return FALSE;
        }
    }
    return TRUE;
}

void PrintThresholds(EpsThreshVolt_t thresholds) {
    printf("Vdown_safe: %d\n", thresholds.fields.Vdown_safe);
    printf("Vdown_cruise: %d\n", thresholds.fields.Vdown_cruise);
    printf("Vdown_full: %d\n", thresholds.fields.Vdown_full);
    printf("Vup_safe: %d\n", thresholds.fields.Vup_safe);
    printf("Vup_cruise: %d\n", thresholds.fields.Vup_cruise);
    printf("Vup_full: %d\n", thresholds.fields.Vup_full);
}

Boolean CompareThresholds(EpsThreshVolt_t thresholds1, EpsThreshVolt_t thresholds2) {
    for (int i = 0; i < NUMBER_OF_THRESHOLD_VOLTAGES; ++i) {
        if (thresholds1.raw[i] != thresholds2.raw[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

Boolean TestThresholdChanges() {
    printf("Getting threshold values \n");
    EpsThreshVolt_t initial_thresh;
    if (GetThresholdVoltages(&initial_thresh) != 0) {
        printf("Failed to get threshold values \n");
        return FALSE;
    } else {
        PrintThresholds(initial_thresh);
    }

    printf("Updating threshold values \n");
    EpsThreshVolt_t new_thresh = {(voltage_t)1000, (voltage_t)2000, (voltage_t)3000,	 \
										  (voltage_t)800, (voltage_t)1800, (voltage_t)2800};
    if (UpdateThresholdVoltages(&new_thresh) != 0) {
        printf("Failed to update threshold values \n");
        return FALSE;
    } else {
        printf("Updated thresholds");
    }

    printf("Getting updated threshold values \n");
    EpsThreshVolt_t new_thresh_ret;
    if (GetThresholdVoltages(&new_thresh_ret) != 0) {
        printf("Failed to get threshold values \n");
        return FALSE;
    } else {
        PrintThresholds(new_thresh_ret);
        if (CompareThresholds(new_thresh, new_thresh_ret) != TRUE) {
            printf("Updated thresholds are incorrect\n");
            return FALSE;
        }
    }

    printf("Restoring default threshold values \n");
    if (RestoreDefaultAlpha() != 0) {
        printf("Failed to restore default threshold values \n");
        return FALSE;
    } else {
        printf("Restored default threshold values \n");
    }

    printf("Getting updated threshold values \n");
    if (GetThresholdVoltages(&new_thresh_ret) != 0) {
        printf("Failed to get threshold values \n");
        return FALSE;
    } else {
        PrintThresholds(new_thresh_ret);
        EpsThreshVolt_t def_thresh = DEFAULT_EPS_THRESHOLD_VOLTAGES;
        if (CompareThresholds(def_thresh, new_thresh_ret) != TRUE) {
            printf("Updated thresholds are incorrect\n");
            return FALSE;
        }
    }

    return TRUE;
}

Boolean TestStateChanges() {
    printf("Performing state machine init\n");
    if (StateMachine_init() != 0) {
        printf("Init failed\n");
        return FALSE;
    }
    if (GetSystemState() != Startup) {
        printf("Init did not set state to startup\n");
        return FALSE;
    }

    printf("Entering critical mode manually\n");
    if (EnterCriticalMode() != 0) {
        printf("Enter critical mode failed\n");
        return FALSE;
    }
    if (GetSystemState() != CriticalMode) {
        printf("Entering critical mode manually did not set state correctly\n");
        return FALSE;
    }

    EpsThreshVolt_t thresh;
    GetThresholdVoltages(&thresh);

    printf("Updating state based on voltage above Vup_safe \n");
    if (ChangeStateByVoltage(thresh.fields.Vup_safe + 1) != 0) {
        printf("ChangeStateByVoltage failed\n");
        return FALSE;
    }
    if (GetSystemState() != SafeMode) {
        printf("Did not enter safe mode correctly\n");
        return FALSE;
    }

    printf("Updating state based on voltage above Vup_cruise \n");
    if (ChangeStateByVoltage(thresh.fields.Vup_cruise + 1) != 0) {
        printf("ChangeStateByVoltage failed\n");
        return FALSE;
    }
    if (GetSystemState() != CruiseMode) {
        printf("Did not enter cruise mode correctly\n");
        return FALSE;
    }

    printf("Updating state based on voltage above Vup_full \n");
    if (ChangeStateByVoltage(thresh.fields.Vup_full + 1) != 0) {
        printf("ChangeStateByVoltage failed\n");
        return FALSE;
    }
    if (GetSystemState() != FullMode) {
        printf("Did not enter full mode correctly\n");
        return FALSE;
    }

    printf("Updating state based on voltage below Vdown_full \n");
    if (ChangeStateByVoltage(thresh.fields.Vdown_full - 1) != 0) {
        printf("ChangeStateByVoltage failed\n");
        return FALSE;
    }
    if (GetSystemState() != CruiseMode) {
        printf("Did not enter cruise mode correctly\n");
        return FALSE;
    }

    printf("Updating state based on voltage below Vdown_cruise \n");
    if (ChangeStateByVoltage(thresh.fields.Vdown_cruise - 1) != 0) {
        printf("ChangeStateByVoltage failed\n");
        return FALSE;
    }
    if (GetSystemState() != SafeMode) {
        printf("Did not enter safe mode correctly\n");
        return FALSE;
    }

    printf("Updating state based on voltage below Vdown_safe \n");
    if (ChangeStateByVoltage(thresh.fields.Vdown_safe - 1) != 0) {
        printf("ChangeStateByVoltage failed\n");
        return FALSE;
    }
    if (GetSystemState() != CriticalMode) {
        printf("Did not enter critical mode correctly\n");
        return FALSE;
    }

    return TRUE;
}

Boolean TestFilterAndConditioning() {
    SetVoltage(8000);
    voltage_t cur_filtered_voltage = filtered_voltage;
    printf("filtered voltage: %d", cur_filtered_voltage);
    printf("state: %d", GetSystemState());
    for (int i = 0; i < 10; ++i) {
        if (EPS_Conditioning() != 0) {
            printf("Conditioning failed\n");
            return FALSE;
        }
        if (filtered_voltage <= cur_filtered_voltage) {
            printf("Conditioning didn't update voltage correctly failed\n");
            return FALSE;
        }
        cur_filtered_voltage = filtered_voltage;
        printf("filtered voltage: %d", cur_filtered_voltage);
        printf("state: %d", GetSystemState());
    }

    SetVoltage(1000);
    for (int i = 0; i < 10; ++i) {
        if (EPS_Conditioning() != 0) {
            printf("Conditioning failed\n");
            return FALSE;
        }
        if (filtered_voltage >= cur_filtered_voltage) {
            printf("Conditioning didn't update voltage correctly failed\n");
            return FALSE;
        }
        cur_filtered_voltage = filtered_voltage;
        printf("filtered voltage: %d", cur_filtered_voltage);
        printf("state: %d", GetSystemState());
    }
    return TRUE;
}

Boolean MainEpsTestBench() {
    Boolean alpha_changes_success = TestAlphaChanges();
    Boolean thresh_changes_success = TestThresholdChanges();
    Boolean state_changes_success = TestStateChanges();
    Boolean filter_success = TestFilterAndConditioning();
    printf("TestAlphaChanges: %s\n", alpha_changes_success ? "SUCCESS" : "FAIL");
    printf("TestThresholdChanges: %s\n", thresh_changes_success ? "SUCCESS" : "FAIL");
    printf("TestStateChanges: %s\n", state_changes_success ? "SUCCESS" : "FAIL");
    printf("TestFilterAndConditioning: %s\n", filter_success ? "SUCCESS" : "FAIL");
    return TRUE;
}
