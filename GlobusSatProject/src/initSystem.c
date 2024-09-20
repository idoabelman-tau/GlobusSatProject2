#include "initSystem.h"
#include "GlobalStandards.h"
 

/*!
*  @brief Calls the FRAM_start functions provided with the satliet
*  @return Returs 0 upon successful initialization of FRAM, else -1.
*/
int StartFRAM(){
	int flag = FRAM_start();
    if(flag != E_NO_SS_ERR){
        logError(init_system, __LINE__, flag, "FRAM did not initialize");
        return flag;
    }
    return 0;

}
/*!
*  @brief This function writes default values of the different sub-systems into the FRAM. 
*         This, essentially, is a commitment to use these addresses for the specified value throughout the run.
*  @example FRAM at SECONDS_SINCE_DEPLOY_ADDR address will hold the amount of seconds since the deploy function was called.    
*  @return returns 0 if all writes to FRAM were successful, else -1.
*/
void WriteDefaultValuesToFRAM(){
	time_unix default_no_comm_thresh;
	default_no_comm_thresh = DEFAULT_NO_COMM_WDT_KICK_TIME;
	FRAM_write((unsigned char*) &default_no_comm_thresh , NO_COMM_WDT_KICK_TIME_ADDR , NO_COMM_WDT_KICK_TIME_SIZE);

	EpsThreshVolt_t def_thresh_volt = { .raw = DEFAULT_EPS_THRESHOLD_VOLTAGES};
	FRAM_write((unsigned char*)def_thresh_volt.raw, EPS_THRESH_VOLTAGES_ADDR,
	EPS_THRESH_VOLTAGES_SIZE);

	float def_alpha;
	def_alpha = DEFAULT_ALPHA_VALUE;
	FRAM_write((unsigned char*) &def_alpha ,EPS_ALPHA_FILTER_VALUE_ADDR , EPS_ALPHA_FILTER_VALUE_SIZE);

	time_unix tlm_save_period = DEFAULT_EPS_SAVE_TLM_TIME;
	FRAM_write((unsigned char*) &tlm_save_period, EPS_SAVE_TLM_PERIOD_ADDR,
			sizeof(tlm_save_period));

	tlm_save_period = DEFAULT_TRXVU_SAVE_TLM_TIME;
	FRAM_write((unsigned char*) &tlm_save_period, TRXVU_SAVE_TLM_PERIOD_ADDR,
			sizeof(tlm_save_period));

	tlm_save_period = DEFAULT_ANT_SAVE_TLM_TIME;
	FRAM_write((unsigned char*) &tlm_save_period, ANT_SAVE_TLM_PERIOD_ADDR,
			sizeof(tlm_save_period));

	tlm_save_period = DEFAULT_SOLAR_SAVE_TLM_TIME;
	FRAM_write((unsigned char*) &tlm_save_period, SOLAR_SAVE_TLM_PERIOD_ADDR,
			sizeof(tlm_save_period));

	tlm_save_period = DEFAULT_WOD_SAVE_TLM_TIME;
	FRAM_write((unsigned char*) &tlm_save_period, WOD_SAVE_TLM_PERIOD_ADDR,
			sizeof(tlm_save_period));

	time_unix beacon_interval = 0;
	beacon_interval = DEFAULT_BEACON_INTERVAL_TIME;
	FRAM_write((unsigned char*) &beacon_interval, BEACON_INTERVAL_TIME_ADDR,
			BEACON_INTERVAL_TIME_SIZE);

	short rssi;
	rssi = DEFAULT_RSSI_VALUE;
	FRAM_write((unsigned char*) &rssi ,TRANSPONDER_RSSI_ADDR , TRANSPONDER_RSSI_SIZE);

	// set the reset counter to zero
	unsigned int num_of_resets = 0;
	FRAM_write((unsigned char*) &num_of_resets,
	NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE);

	FRAM_write((unsigned char*) &num_of_resets,
	NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_SIZE);

	FRAM_write((unsigned char*) &num_of_resets,
	TRANSPONDER_END_TIME_ADDR, TRANSPONDER_END_TIME_SIZE);

	FRAM_write((unsigned char*) &num_of_resets,
	DEL_OLD_FILES_NUM_DAYS_ADDR, DEL_OLD_FILES_NUM_DAYS_SIZE);

	Boolean flag = FALSE;
	FRAM_write((unsigned char*) &flag,
			STOP_REDEPOLOY_FLAG_ADDR, STOP_REDEPOLOY_FLAG_SIZE);

	ResetGroundCommWDT();

	setMuteEndTime(0);

}

/*!
*@note I2C is a basic communication protocol. It allows more than one master
*@return returns 0 upon successful initialization of I2C, else -1.
*/
int StartI2C(){
	int flag = I2C_start(BUS_SPEED,BUS_TIMEOUT);
    if(flag != 0){
        logError(init_system, __LINE__, flag, "failed to initialize I2C");
        return flag;
    }
    return 0;
}


int StartSPI(){
	int flag = SPI_start(both_spi, slave1_spi);
    if(flag != 0){ // might want to change the slave in this init. might be ignored anyway.
        logError(init_system, __LINE__, flag, "failed to initialize SPI");
        return flag;
    } 
    return 0;
}

int StartTIME(){
	const Time default_time = UNIX_DATE_JAN_D1_Y2000;
	int flag = Time_start(&default_time,0);
    if(flag != 0){
    	logError(init_system, __LINE__, flag, "failed to initialize time");
        return flag;
    }

    time_unix unixtime;
	int err = FRAM_read((unsigned char *)&unixtime, MOST_UPDATED_SAT_TIME_ADDR, MOST_UPDATED_SAT_TIME_SIZE);
	if (err != 0) {
		logError(init_system, __LINE__, flag, "failed to read time from RAM");
		return err;
	}
	if (unixtime > UNIX_SECS_FROM_Y1970_TO_Y2000) {
		Time_setUnixEpoch(unixtime);
	}

	ResetGroundCommWDT();

    return 0;
}

int DeploySystem(){

	logError(init_system, __LINE__, INFO_MSG, "Deploy first activation");
	// write default values to FRAM
	WriteDefaultValuesToFRAM();
	TelemetryCollectorLogic(); // start telemnetry collector before deploy

	// if we are here...it means we are in the first activation, wait 30min, deploy and make firstActivation flag=false
	int err = 0;

	time_unix seconds_since_deploy = 0;
	err = FRAM_read((unsigned char*) &seconds_since_deploy , SECONDS_SINCE_DEPLOY_ADDR , SECONDS_SINCE_DEPLOY_SIZE);
	if (err != E_NO_SS_ERR) {
		seconds_since_deploy = 0;
	}

	time_unix startTime = 0;
	Time_getUnixEpoch(&startTime);
	startTime -= seconds_since_deploy;

	// wait 30 min + log telm
	while (seconds_since_deploy < MINUTES_TO_SECONDS(MIN_2_WAIT_BEFORE_DEPLOY)) { // RBF to 30 min
		logError(init_system, __LINE__, INFO_MSG,"Deploy wait loop start");
		// wait 10 sec and update timer in FRAM
		vTaskDelay(SECONDS_TO_TICKS(10));

		time_unix currTime = 0;
		Time_getUnixEpoch(&currTime);
		seconds_since_deploy = currTime - startTime;

		FRAM_write((unsigned char*)&seconds_since_deploy, SECONDS_SINCE_DEPLOY_ADDR,
				SECONDS_SINCE_DEPLOY_SIZE);

		// reset EPS WDT
		imepsv2_piu__replyheader_t eps_cmd;
		imepsv2_piu__resetwatchdog(EPS_I2C_BUS_INDEX, &eps_cmd);

	}
	logError(init_system, __LINE__, INFO_MSG,"Deploy wait loop - DONE");

	// open ants !
	//CMD_AntennaDeploy(NULL);

	// set deploy time in FRAM
	time_unix deploy_time = 0;
	Time_getUnixEpoch(&deploy_time);
	FRAM_write((unsigned char*) &deploy_time, DEPLOYMENT_TIME_ADDR,
			DEPLOYMENT_TIME_SIZE);

	// set first activation false in FRAM
	Boolean first_activation = FALSE;
	FRAM_write((unsigned char*) &first_activation,
			FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE);

	return 0;
}

int InitSubsystems(){
    StartI2C();
    StartFRAM();
    StartTIME();
    InitializeFS();
    EPS_Init();
    InitTrxvu();

#ifdef TESTING
    WriteDefaultValuesToFRAM();
    TelemetryCollectorLogic();
    EnterFullMode(); // start from full mode for testing
#else
	Boolean first_activation;
	FRAM_read((unsigned char*) &first_activation,
				FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE);
	if (first_activation) {
		DeploySystem(); // DeploySystem includes starting telemetry collection
	} else {
		TelemetryCollectorLogic();
	}
    EnterSafeMode();
#endif

    return 0;
}
