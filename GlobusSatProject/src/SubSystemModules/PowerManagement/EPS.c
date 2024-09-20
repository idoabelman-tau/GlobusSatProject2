/*
 * @file	EPS.c
 * @brief	EPS- Energy Powering System.This system is incharge of the energy consumtion
 * 			the satellite and switching on and off power switches(5V, 3V3)
 * @see		inspect logic flowcharts thoroughly in order to write the code in a clean manner.
 */
#include "EPS.h"

#define EPS_I2C_ADDRESS 0x20 //TODO: make sure this address (in demo) is hard coded in eps

/* Global variables*/
voltage_t filtered_voltage;


int EPS_Init() {

    // initialize drivers and solar panel
	int SPI_ERR_FLAG = StartSPI();
	if(SPI_ERR_FLAG != E_NO_SS_ERR){
		logError(eps, __LINE__, SPI_ERR_FLAG, "failed to init SPI");

		return SPI_ERR_FLAG;
	}

	int EPS_ERR_FLAG;
#ifdef ISISEPS
	IMEPSV2_PIU_t EPS_INIT_STRUCT;
	EPS_INIT_STRUCT.i2cAddr = EPS_I2C_ADDRESS;

	EPS_ERR_FLAG = IMEPSV2_PIU_Init(&EPS_INIT_STRUCT, 1);

	if( EPS_ERR_FLAG != driver_error_reinit && EPS_ERR_FLAG != driver_error_none)
	// re-initialization is not an error
	{
		logError(eps, __LINE__, EPS_ERR_FLAG, "failed to init EPS");
		return EPS_ERR_FLAG;
	}
#endif
#ifdef GOMEPS
	unsigned char eps_i2c_addr = EPS_I2C_ADDR;
	EPS_ERR_FLAG = GomEpsInitialize(&eps_i2c_addr, 1);
	EPS_ERR_FLAG += GomEpsPing(EPS_I2C_BUS_INDEX, eps_i2c_addr, &eps_i2c_addr);
	if( EPS_ERR_FLAG != E_IS_INITIALIZED && EPS_ERR_FLAG != E_NO_SS_ERR)
	// re-initialization is not an error
	{
		logError(eps, __LINE__, EPS_ERR_FLAG, "failed to init EPS");
		return EPS_ERR_FLAG;
	}
#endif


    filtered_voltage = 0;

    return EPS_Conditioning();

}

int update_filtered_voltage() {
    voltage_t vbat = 0;
    int err = GetBatteryVoltage(&vbat);
    if (err != 0) {
    	logError(eps, __LINE__, err, "failed to get battery voltage");
        return err;
    }
    float alpha;
    err = GetAlpha(&alpha);
    if (err != 0) {
    	logError(eps, __LINE__, err, "failed to get alpha");
        return err;
    }

    filtered_voltage = alpha * vbat + (1-alpha) * filtered_voltage;

    return 0;
}

int EPS_Conditioning() {
	int err = update_filtered_voltage();
    if (err != 0) {
    	logError(eps, __LINE__, err, "failed to update filtered voltage");
        return err;
    }
    return ChangeStateByVoltage(filtered_voltage);
}

int GetBatteryVoltage_isis(voltage_t *vbat) {
    imepsv2_piu__gethousekeepingeng__from_t response;

    int error = imepsv2_piu__gethousekeepingeng(0,&response);
    if (error) {
    	logError(eps, __LINE__, error, "failed to get isis housekeeping data");
        return error;
    }

    *vbat = response.fields.batt_input.fields.volt;
    return 0;
}

int GetBatteryVoltage_gom(voltage_t *vbat) {
	gom_eps_hk_t myEpsTelemetry_hk;
	int error = GomEpsGetHkData_general(0, &myEpsTelemetry_hk);
	if (error != E_NO_SS_ERR) {
    	logError(eps, __LINE__, error, "failed to get gom housekeeping data");
		return error;
	}

    *vbat = myEpsTelemetry_hk.fields.vbatt;
    return 0;
}

int GetAlpha(float *alpha) {
    if (alpha == NULL) {
    	logError(eps, __LINE__, E_INPUT_POINTER_NULL, "alpha pointer is null");
        return E_INPUT_POINTER_NULL;
    }

    int err = FRAM_read((unsigned char*) alpha, EPS_ALPHA_FILTER_VALUE_ADDR, EPS_ALPHA_FILTER_VALUE_SIZE);
    if (err != 0) {
    	logError(eps, __LINE__, err, "failed to read FRAM");

        return err;
    }
    return 0;
}

int UpdateAlpha(float new_alpha) {
    if (new_alpha <= 0.0 || new_alpha > 1.0) {
    	logError(eps, __LINE__, E_PARAM_OUTOFBOUNDS, "alpha pointer is null");
        return E_PARAM_OUTOFBOUNDS;
    }

    int err = FRAM_write((unsigned char*) &new_alpha, EPS_ALPHA_FILTER_VALUE_ADDR, EPS_ALPHA_FILTER_VALUE_SIZE);

    if (err != 0) {
		logError(eps, __LINE__, err, "failed to write FRAM");

		return err;
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
;

