/*
 * TM_managment.h
 *
 *  Created on: Apr 8, 2019
 *      Author: Hoopoe3n
 */

#ifndef TM_MANAGMENT_H_
#define TM_MANAGMENT_H_

#include "utils.h"
#include <stddef.h>
#include <stdlib.h>
//#include <stdio.h>
#include <string.h>
#include <time.h>

/*
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>*/
#include <hal/Timing/Time.h>
#include <hal/Boolean.h>

#include <satellite-subsystems/IsisSolarPanelv2.h>
#include <satellite-subsystems/isis_ants2.h>
#include <satellite-subsystems/GomEPS.h>


#include <freertos/FreeRTOSConfig.h>

#include "SubSystemModules/Housekeeping/TelemetryFiles.h"

#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "TelemetryCollector.h"

#include "hal/errors.h"
#define NUMBER_OF_LOG_TYPES 14

#define MAX_FILE_SIZE 100 //TODO: maybe change this into 8*512/(avrage entry size).(about 157 for eps might be optimal)

#ifdef ISISEPS
#define EPS_TELEM_SIZE sizeof(imepsv2_piu__gethousekeepingeng__from_t)
#endif
#ifdef GOMEPS
#define EPS_TELEM_SIZE sizeof(gom_eps_hk_t)
#endif





typedef enum
{
	FS_SUCCSESS,
	FS_DUPLICATED,
	FS_LOCKED,
	FS_TOO_LONG_NAME,
	FS_BUFFER_OVERFLOW,
	FS_NOT_EXIST,
	FS_ALLOCATION_ERROR,
	FS_FRAM_FAIL,
	FS_FAT_API_FAIL,
	FS_FAIL
} FileSystemResult;

// used to write log files for easy search of data.
//fields[0] - number of entries in file
//fields[1] - name of file
//fields[2] - sample time first line in file
//fields[3] - sample time last line in file

typedef union _logElement{
	int fields[4];
	unsigned char raw[16];
} logElement;

typedef union _SolarPack{
	unsigned char raw[44];
	struct {
		int32_t temp;
		int8_t status;
		int state;
	} parsed;
} SolarPack;


#define _SD_CARD 0
/*!
 * Initializes the file system.
 * @note call once for boot and after DeInitializeFS.
 * @return FS_FAIL if Initializing the FS failed,
 * FS_ALLOCATION_ERROR on malloc error,
 * FS_SUCCSESS on success.
 */
FileSystemResult InitializeFS();

/*!
 * DeInitializes the file system.
 */
void DeInitializeFS(int sd_card);


/// simple function by daniel:

void WriteData(tlm_type_t tlm  ,unsigned char* data);

void findData(tlm_type_t tlm, unsigned int from , unsigned int to);

void zeroize();


void deleteData(tlm_type_t tlm , unsigned int from , unsigned int to);

void hex_print(unsigned char* text , int length);

void deleteOldDays(tlm_type_t tlm, int numberOfDays);

#endif /* TM_MANAGMENT_H_ */




