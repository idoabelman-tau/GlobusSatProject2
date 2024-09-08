#include "Maintenance.h"

int DeleteOldFiles(int minFreeSpace) {
	return 0;
}

Boolean IsFS_Corrupted() {
	return FALSE;
}

Boolean IsGroundCommunicationWDTKick() {
	time_unix last_comm_time;
	int err = FRAM_read((unsigned char*) &last_comm_time, LAST_COMM_TIME_ADDR, LAST_COMM_TIME_SIZE);
	if (err != E_NO_SS_ERR) {
		return TRUE; // kick watchdog if we're unable to read the saved time
	}

	time_unix no_comm_wdt_kick_time;
	err = FRAM_read((unsigned char*) &no_comm_wdt_kick_time, NO_COMM_WDT_KICK_TIME_ADDR, NO_COMM_WDT_KICK_TIME_SIZE);
	if (err != E_NO_SS_ERR) {
		return TRUE; // kick watchdog if we're unable to read the saved time
	}

	Time time;
	Time_get_wrap(&time);
	time_unix unixtime = Time_convertTimeToEpoch(&time);
	if (unixtime - last_comm_time > no_comm_wdt_kick_time) {
		return TRUE;
	}

	return FALSE;
}

/*!
 * @brief reads the current UNIX time and writes it into the FRAM for future reference.
 */
void SaveSatTimeInFRAM(unsigned int time_addr, unsigned int time_size) {
	Time time;
	Time_get_wrap(&time);
	time_unix unixtime = Time_convertTimeToEpoch(&time);
	FRAM_write((unsigned char *)&unixtime, MOST_UPDATED_SAT_TIME_ADDR, MOST_UPDATED_SAT_TIME_SIZE);
}

/*!
 * @brief Calls the relevant functions in a serial order
 */
void Maintenance() {
	DeleteOldFiles(MIN_FREE_SPACE);

	/*if (IsGroundCommunicationWDTKick()) {
		restart();
	}*/
}

