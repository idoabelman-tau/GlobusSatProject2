/*
 * utils.c
 *
 *  Created on: Sep 19, 2019
 *      Author: pc
 */

#include "utils.h"

Time ref = UNIX_DATE_JAN_D1_Y2000;
Boolean realTime = FALSE;

void set_real_time(Boolean value) {
	realTime = value;
}

void set_ref(Time t){
	ref. year = t.year;
	ref.month = t.month;
	ref.date = t.date;
	ref.seconds = t.seconds;
	ref.day = t.day;
	ref.hours = t.hours;
	ref.minutes = t.minutes;
	ref.secondsOfYear = t.secondsOfYear;
}
void set_ref_epoch(unsigned int epoch){
	Time_convertEpochToTime(epoch, &ref);
}
void reset_epoch(){
	set_ref_epoch(UNIX_SECS_FROM_Y1970_TO_Y2000);
}

int Time_get_stub(Time* t){
	if (realTime) {
		return Time_get(t);
	}
	else {
		unsigned int offset = 300;
		Time_convertEpochToTime(Time_convertTimeToEpoch(&ref) + offset, &ref);
		*t =ref;
		return 0;
	}
}

int logError(fileEnum file, int line, int errorNum, char* msg) {
//#ifdef TESTING
	printf("file: %d, line: %d, error num: %d, message: %s\n", file, line, errorNum, msg);
//#endif

	errorElement err = {file, line, errorNum};
	WriteData(tlm_log,(unsigned char*) &err);
	return 0;

}

