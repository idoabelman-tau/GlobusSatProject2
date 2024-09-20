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
		logError(maintenance, __LINE__, err, "failed to read FRAM");
		return TRUE; // kick watchdog if we're unable to read the saved time
	}

	time_unix no_comm_wdt_kick_time;
	err = FRAM_read((unsigned char*) &no_comm_wdt_kick_time, NO_COMM_WDT_KICK_TIME_ADDR, NO_COMM_WDT_KICK_TIME_SIZE);
	if (err != E_NO_SS_ERR) {
		logError(maintenance, __LINE__, err, "failed to read FRAM");
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
	int err = FRAM_write((unsigned char *)&unixtime, time_addr, time_size);
	if (err != E_NO_SS_ERR) {
		logError(maintenance, __LINE__, err, "failed to write to FRAM");
	}
}

/*!
 * @brief Calls the relevant functions in a serial order
 */
void Maintenance() {
	DeleteOldFiles(MIN_FREE_SPACE);
	SaveSatTimeInFRAM(MOST_UPDATED_SAT_TIME_ADDR, MOST_UPDATED_SAT_TIME_SIZE);

	/*if (IsGroundCommunicationWDTKick()) {
		restart();
	}*/

	// kick eps watchdog
	imepsv2_piu__replyheader_t eps_cmd;
	imepsv2_piu__resetwatchdog(EPS_I2C_BUS_INDEX, &eps_cmd);
}

