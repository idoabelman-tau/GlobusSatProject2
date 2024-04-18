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

int DeploySystem(){
	return 1;
}

int InitSubsystems(){
    StartI2C();
    StartFRAM();
    StartTIME();
    InitializeFS();
    EPS_Init();
    //InitTrxvu();
    return 0;
}
