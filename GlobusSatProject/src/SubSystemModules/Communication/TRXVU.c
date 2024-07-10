#include "TRXVU.h"
#include "SubSystemModules/Housekeeping/TelemetryCollector.h"
#include "SatCommandHandler.h"

#define RECIEVER_I2C_ADDRESS 0x60
#define TRANSMITTER_I2C_ADDRESS 0x61

Boolean TrxMuted = FALSE;

int InitTrxvu(){
	//definitions

	ISIStrxvuI2CAddress TRX_I2C_ADDR_STRUCT;
	ISIStrxvuFrameLengths TRX_FRAME_LENGTH_STRUCT;
	ISIStrxvuBitrate TRX_BIT_RATE_STRUCT;
	int TRX_ERR_FLAG;

	//set TRX default values
	TRX_I2C_ADDR_STRUCT.addressVu_rc = RECIEVER_I2C_ADDRESS;
	TRX_I2C_ADDR_STRUCT.addressVu_tc = TRANSMITTER_I2C_ADDRESS;

	TRX_FRAME_LENGTH_STRUCT.maxAX25frameLengthTX = SIZE_TXFRAME;
	TRX_FRAME_LENGTH_STRUCT.maxAX25frameLengthRX = SIZE_RXFRAME;

	TRX_BIT_RATE_STRUCT = trxvu_bitrate_9600;  //TODO: set correct bit

	// initialize
	TRX_ERR_FLAG = IsisTrxvu_initialize(&TRX_I2C_ADDR_STRUCT,&TRX_FRAME_LENGTH_STRUCT,&TRX_BIT_RATE_STRUCT,1);

	if(TRX_ERR_FLAG != E_NO_SS_ERR && TRX_ERR_FLAG != E_IS_INITIALIZED){
		//TODO: log error
		return  TRX_ERR_FLAG;
	}

	return E_NO_SS_ERR;
}

int TRX_Logic() {
	if (CheckForMuteEnd()) {
		UnMuteTRXVU();
	}

	sat_packet_t cmd;
	int err = GetOnlineCommand(&cmd);

	/*if(err == no_command_found) {
		err = GetDelayedCommand(sat_packet_t *cmd);
	}*/ // delayed commands not currently implemented

	if (err == E_NO_SS_ERR) { // a command was found either in online or delayed buffer
		ResetGroundCommWDT();
		SendAckPacket(ACK_RECEIVE_COMM, &cmd, NULL, 0);
		err = ActUponCommand(&cmd);
	}
	else if (err != E_NO_COMMAND_FOUND) {
		// TODO: log error
		printf("error getting command\n");
	}

	BeaconLogic();

	return err;
}

Boolean CheckTransmitionAllowed() {
	return !TrxMuted && GetSystemState != SafeMode;
}

int TransmitSPLPacket(sat_packet_t *packet, int *avalFrames) {
	unsigned char packet_length = sizeof(sat_packet_t) - sizeof(packet->data) + packet->length; // only submit the actual data
	if (CheckTransmissionAllowed()) {
		int err = IsisTrxvu_tcSendAX25DefClSign(0, (unsigned char *)packet, packet_length, (unsigned char *)avalFrames);

		if(err != E_NO_SS_ERR){
			printf("error sending packet\n");
			//TODO: log error
			return  err;
		}
	}
	else {
		return E_CANT_TRANSMIT;
	}
}

int AssembleAndSendPacket(unsigned char *data, unsigned short data_length, char type, char subtype,unsigned int id) {
	sat_packet_t packet;
	int err = AssembleSPLPacket(data, data_length, type, subtype, id, &packet);
	if (err != E_NO_SS_ERR) {
		// TODO: log error
		return -1;
	}

	err = TransmitSPLPacket(&packet, NULL);
	if (err != E_NO_SS_ERR) {
		// TODO: log error
		return -1;
	}
	return 0;
}

void ResetGroundCommWDT() {
	Time time;
	Time_get(Time *time);
	time_unix unixtime = Time_convertTimeToEpoch(time);
	FRAM_write((unsigned char*) &unixtime, LAST_COMM_TIME_ADDR, LAST_COMM_TIME_SIZE);
}

int BeaconLogic() {
	static unsigned int last_sent_time = 0; // last time the beacon was sent, in uptime seconds
	unsigned int interval;

	int err = GetBeaconInterval(&interval);
	if (err != 0) {
		//TODO: log error
		return err;
	}

	unsigned int cur_time = Time_getUptimeSeconds();

	if (cur_time - last_sent_time >= interval) { // more than interval seconds have passed since the last time the beacon was sent, send a new one
		printf("interval has passed, sending beacon\n");
		last_sent_time = Time_getUptimeSeconds();
		return SendBeacon();
	}

	return 0;
}

int SendBeacon() {

	WOD_Telemetry_t wod;
	GetCurrentWODTelemetry(&wod);

	int err = AssembleAndSendPacket((unsigned char *)&wod, sizeof(wod), trxvu_cmd_type, BEACON_SUBTYPE, YCUBE_SAT_ID);
	if (err != 0) {
		// TODO: log error
		return -1;
	}

	return 0;
}

int GetOnlineCommand(sat_packet_t *cmd) {

	unsigned short RxCounter = 0;
	unsigned char rxframebuffer[SIZE_RXFRAME] = {0};
	ISIStrxvuRxFrame rxFrameCmd = {0,0,0, rxframebuffer};

	int err = IsisTrxvu_rcGetFrameCount(0, &RxCounter);
	if (err != E_NO_SS_ERR) {
		// TODO: log error
		return err;
	}

	while(RxCounter > 0)
	{
		int err = IsisTrxvu_rcGetCommandFrame(0, &rxFrameCmd);
		if (err != E_NO_SS_ERR) {
			// TODO: log error
			return err;
		}

		err = ParseDataToSPLPacket(rxFrameCmd.rx_framedata, cmd);
		int sat_id = GetSatId(cmd);
		if (err == E_NO_SS_ERR && (sat_id == YCUBE_SAT_ID || sat_id == ALL_SAT_ID)) {
			return err;
		}

		err = IsisTrxvu_rcGetFrameCount(0, &RxCounter);
		if (err != E_NO_SS_ERR) {
			// TODO: log error
			return err;
		}

	}

	return E_NO_COMMAND_FOUND;
}

int GetBeaconInterval(unsigned int *interval) {
    if (interval == NULL) {
        return -1;
    }

    int err = FRAM_read((unsigned char*) &interval, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE);
    if (err != 0) {
        return -1;
    }
    return 0;
}

int SetBeaconInterval(unsigned int *interval) {
    if (*interval < MIN_BEACON_INTERVAL || *interval > MAX_BEACON_INTERVAL) {
        return -1;
    }

    int err = FRAM_write((unsigned char*) interval, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE);

    if (err != 0) {
        return -2;
    }
    return 0;
}

int RestoreDefaultBeaconInterval() {
	int interval = DEFAULT_BEACON_INTERVAL_TIME;
    return SetBeaconInterval(&interval);
}

int SendAckPacket(ack_subtype_t acksubtype, sat_packet_t *cmd, unsigned char *data, unsigned short length) {
	int err = AssembleAndSendPacket(data, length, ack_type, acksubtype, cmd->ID);
	if (err != 0) {
		// TODO: log error
		return -1;
	}

	return 0;
}

int setMuteEndTime(time_unix *endTime) {
	int err = FRAM_write((unsigned char*) endTime, MUTE_END_TIME_ADDR, MUTE_END_TIME_SIZE);

	return err;
}

int getMuteEndTime(time_unix *endTime) {
	int err = FRAM_read((unsigned char*) endTime, MUTE_END_TIME_ADDR, MUTE_END_TIME_SIZE);

	return err;
}

int SetIdleState(ISIStrxvuIdleState state, time_unix duration) {
	return IsisTrxvu_tcSetIdlestate(0, state);
}

int muteTRXVU(time_unix duration) {
	Time time;
	Time_get(Time *time);
	time_unix unixtime = Time_convertTimeToEpoch(time);
	setMuteEndTime(unixtime + duration);
	TrxMuted = TRUE;
}

void UnMuteTRXVU() {
	Time time;
	Time_get(Time *time);
	time_unix unixtime = Time_convertTimeToEpoch(time);
	setMuteEndTime(unixtime); // set the mute end time to now so that even if we get muted by a bit flip we will automatically unmute
	TrxMuted = FALSE;
}

Boolean CheckForMuteEnd() {
	Time time;
	Time_get(Time *time);
	time_unix unixtime = Time_convertTimeToEpoch(time);
	time_unix endtime;
	getMuteEndTime(&endtime);
	return unixtime > endtime;
}
