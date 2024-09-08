/*
 * utils.c
 *
 *  Created on: Sep 19, 2019
 *      Author: pc
 */

#include "utils.h"

#ifdef TESTING
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

int Time_get_stub(Time* t){
	if (realTime) {
		return Time_get(t);
	}
	else {
		unsigned int offset = rand()%40 + 3;
		Time_convertEpochToTime(Time_convertTimeToEpoch(&ref) + offset, &ref);
		*t =ref;
		return 0;
	}
}
#endif

