/*
 *
 * @file	InitSystem.h
 * @brief	All crucial initialization functions in one place
 * @note	Order of function calls is important. Read system documents for further analysis
 */

#include "hal/Storage/FRAM.h"
#include "FRAM_FlightParameters.h"
#include "hal/Drivers/I2C.h"
#include "hal/Drivers/SPI.h"
#include "hal/Timing/Time.h"
#include "hal/Timing/WatchDogTimer.h"
#include "SubSystemModules/PowerManagement/EPS.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "GlobalStandards.h"
#include "TLM_management.h"
#include <stddef.h>

#ifndef INITSYSTEM_H_
#define INITSYSTEM_H_


#define MIN_2_WAIT_BEFORE_DEPLOY 45 // how many minutes to wait before we open the Ants TODO: before flight change to 30
#define RESTART_TIME 3 // how much time does it take to restart the SAT

#define BUS_SPEED 10000
#define BUS_TIMEOUT 10 // all bytes are transformed in 0.1*BUS_TIMEOUT ticks. timeout otherwise.


/*!
 * @brief	Starts the FRAM using drivers, and checks for errors.
 * @see FRAM.h
 */
int StartFRAM();

/*!
 * @brief	writes the default filght parameters to the corresponding FRAM addresses
 * @see FRAM_FlightParameters.h
 */
void WriteDefaultValuesToFRAM();

/*!
 * @brief	Starts the I2C using drivers, and checks for errors.
 * @see	I2C.h
 */
int StartI2C();


/*!
 * @brief	Starts the SPI using drivers, and checks for errors
 * @see	SPI.h
 */
int StartSPI();


/*!
 * @brief	Starts the Time module using drivers, and checks for errors.
 * @see Time.h
 */
int StartTIME();


/*!
 * @brief	executes all required initializations of systems, including sub-systems, and checks for errors
 * @return	0 successful init
 * 			-1 failed at init
 */
int InitSubsystems();

/*!
 * @brief	deployment procedure
 * @return	0 successful deployment
 * 			-1 failed to deploy
 */
int DeploySystem();

#endif /* INITSYSTEM_H_ */
