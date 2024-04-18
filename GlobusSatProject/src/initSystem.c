#include "initSystem.h"
#include "GlobalStandards.h"
 

/*!
*  @brief Calls the FRAM_start functions provided with the satliet
*  @return Returs 0 upon successful initialization of FRAM, else -1.
*/
int StartFRAM(){
	int flag = FRAM_start();
    if(flag != E_NO_SS_ERR){
       printf("FRAM did not initialize\n");
       // TODO: log error
       return  -1;
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


    /*if(
    FRAM_write('0',DEPLOYMENT_TIME_ADDR,DEPLOYMENT_TIME_SIZE)||

    FRAM_write('0',SECONDS_SINCE_DEPLOY_ADDR,SECONDS_SINCE_DEPLOY_SIZE)||

    FRAM_write('0',NO_COMM_WDT_KICK_TIME_ADDR,NO_COMM_WDT_KICK_TIME_SIZE)||

    FRAM_write('0',STOP_REDEPOLOY_FLAG_ADDR,STOP_REDEPOLOY_FLAG_SIZE)||

    FRAM_write('0',MUTE_END_TIME_ADDR,MUTE_END_TIME_SIZE)||

    FRAM_write('0',FIRST_ACTIVATION_FLAG_ADDR,FIRST_ACTIVATION_FLAG_SIZE)||

    FRAM_write('0',TRANSPONDER_END_TIME_ADDR,TRANSPONDER_END_TIME_SIZE)||

    FRAM_write('0',TRANSPONDER_RSSI_ADDR,TRANSPONDER_RSSI_SIZE)||

    FRAM_write('0',MOST_UPDATED_SAT_TIME_ADDR,MOST_UPDATED_SAT_TIME_SIZE)||

    FRAM_write('0',NUMBER_OF_RESETS_ADDR,NUMBER_OF_RESETS_SIZE) ||

    FRAM_write('0',RESET_CMD_FLAG_ADDR,RESET_CMD_FLAG_SIZE) ||

    FRAM_write('0',EPS_SAVE_TLM_PERIOD_ADDR,DEFAULT_EPS_SAVE_TLM_TIME) || 

    FRAM_write('0',TRXVU_SAVE_TLM_PERIOD_ADDR,DEFAULT_TRXVU_SAVE_TLM_TIME) ||

    FRAM_write('0',ANT_SAVE_TLM_PERIOD_ADDR,DEFAULT_ANT_SAVE_TLM_TIME) ||

    FRAM_write('0',WOD_SAVE_TLM_PERIOD_ADDR,DEFAULT_WOD_SAVE_TLM_TIME) ||

    FRAM_write('0',NUMBER_OF_CMD_RESETS_ADDR,NUMBER_OF_CMD_RESETS_SIZE) ||

    FRAM_write('0',DELAYED_CMD_FRAME_COUNT_ADDR,DELAYED_CMD_FRAME_COUNT_SIZE) ||

    FRAM_write('0',DEL_OLD_FILES_NUM_DAYS_ADDR,DEL_OLD_FILES_NUM_DAYS_SIZE) ||

    FRAM_write('0',TRANS_ABORT_FLAG_ADDR,TRANS_ABORT_FLAG_SIZE) ||

    FRAM_write('0',EPS_THRESH_VOLTAGES_ADDR, (NUMBER_OF_THRESHOLD_VOLTAGES * sizeof(voltage_t))) ||

    FRAM_write('0',BEACON_INTERVAL_TIME_ADDR,BEACON_INTERVAL_TIME_SIZE)  ||
    
    FRAM_write('0',LAST_COMM_TIME_ADDR,LAST_COMM_TIME_SIZE)
    ){
        printf("failed writing default values\n");
    }*/
}

/*!
*@note I2C is a basic communication protocol. It allows more than one master
*@return returns 0 upon successful initialization of I2C, else -1.
*/
int StartI2C(){
	int flag = I2C_start(BUS_SPEED,BUS_TIMEOUT);
    if(flag != 0){
        printf("failed to initialize I2C");
        // TODO: log error
        return -1;
    }
    return 0;
}


int StartSPI(){
	int flag = SPI_start(both_spi, slave1_spi);
    if(flag != 0){ // might want to change the slave in this init. might be ignored anyway.
        printf("failed to init SPI");
        // TODO: log error
        return -1;
    } 
    return 0;
}

int StartTIME(){
	const Time default_time = UNIX_DATE_JAN_D1_Y2000;
	int flag = Time_start(&default_time,0);
    if(flag != 0){
        printf("failed to initialize time");
        // TODO: log error
        return -1;
    }
    return 0;
}

int InitSubsystems(){
    StartI2C();
    StartFRAM();
    StartTIME();
    InitializeFS();
    //startSPI();
    //InitTrxvu();
    
    return 0;
}
