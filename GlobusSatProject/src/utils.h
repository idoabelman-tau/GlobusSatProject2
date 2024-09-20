/*
 * utils.h
 *
 *  Created on: Sep 19, 2019
 *      Author: pc
 */

#ifndef UTILS_H_
#define UTILS_H_

#include "GlobalStandards.h"
#include <stdlib.h>
#include <hal/Timing/Time.h>
#include "SubSystemModules/Housekeeping/TLM_management.h"


#define E_NO_COMMAND_FOUND -38 // no command found in buffers
#define E_INVALID_SAT_ID -39 // satellite ID in command is invalid
#define E_COMMAND_TYPE_NOT_FOUND -40 // the command type or subtype were not found
#define E_TIMEOUT -41 // timeout in trying to transmit
#define E_CANT_OPEN_FILE -42
#define E_TIMER_UPDATE_FAIL -43
#define E_TIMER_INIT_FAIL -44
#define E_TIMER_START_FAIL -45
#define E_FILE_READ_FAILED -46
#define E_FILE_DELETE_FAILED -47
#define E_CANT_TRANSMIT    		-200
#define E_TOO_EARLY_4_BEACON    -201
#define E_INVALID_PARAMETERS    -204
#define TRXVU_MUTE_TOO_LONG    	-202
#define TRXVU_IDLE_TOO_LONG    	-203
#define TRXVU_TRANSPONDER_TOO_LONG -199
#define TRXVU_IDLE_WHILE_TRANSPONDER -205
#define TRXVU_TRANSPONDER_WHILE_MUTE -211
#define TRXVU_IDEL_WHILE_MUTE        -212
#define BEACON_INTRAVL_TOO_SMALL -206
#define SPL_DATA_TOO_BIG			-207
#define INVALID_TLM_TYPE			-208
#define INVALID_IMG_TYPE			-209
#define INFO_MSG				-210

#define MAX_ERRORS       				 10 // max errors we want to log from the same type toghether
#define MAX_TIME_BETWEEN_ERRORS          60 // max seconds we allow between errors
#define MAX_LOG_STR				100

typedef enum{
	trxvu_commands,
	command_dictionary,
	sat_command_handler,
	trxvu,
	telemetry_collector,
	tlm_management,
	maintenance,
	eps,
	init_system,
	main_file,
	state_machine,
	utils
} fileEnum;

typedef struct {
	fileEnum file;
	int line;
	int errorNum;
} errorElementField;

typedef union _errorElement{
	errorElementField fields;
	unsigned char raw[sizeof(errorElementField)];
} errorElement;


/*
 * convert unix time to Time struct
 */
void timeU2time(time_unix utime, Time *time);
/*
 * log error message
 */
int logError(fileEnum file, int line, int errorNum, char* msg);


void set_real_time(Boolean value);
void set_ref(Time t);
void set_ref_epoch(unsigned int epoch);
int Time_get_stub(Time* t);
void reset_epoch();

#ifdef TESTING
#define Time_get_wrap Time_get_stub
#else
#define Time_get_wrap Time_get
#endif


#endif /* UTILS_H_ */

