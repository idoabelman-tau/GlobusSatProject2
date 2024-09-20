#include "TRXVU.h"
#include "SubSystemModules/Housekeeping/TelemetryCollector.h"
#include "SatCommandHandler.h"

#define RECIEVER_I2C_ADDRESS 0x60
#define TRANSMITTER_I2C_ADDRESS 0x61

Boolean TrxMuted = FALSE;
dump_arguments_t CurrDumpArguments; // global to pass between tasks
Boolean DumpRunning = FALSE;
unsigned char sendBuffer[MAX_COMMAND_DATA_LENGTH];
int dataInSendBuffer = 0;

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
		logError(trxvu, __LINE__, TRX_ERR_FLAG, "error initializing TRXVU");
		return  TRX_ERR_FLAG;
	}

	ISISantsI2Caddress myAntennaAddress;
	myAntennaAddress.addressSideA = ANTS_I2C_SIDE_A_ADDR;
	myAntennaAddress.addressSideB = ANTS_I2C_SIDE_B_ADDR;

	//Initialize the AntS system
	int ants_err = IsisAntS_initialize(&myAntennaAddress, 1);
	if(ants_err != E_NO_SS_ERR && ants_err != E_IS_INITIALIZED){
		logError(trxvu, __LINE__, ants_err, "error initializing ANTS");
		return  ants_err;
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
		logError(trxvu, __LINE__, err, "error getting command");
	}

	BeaconLogic();

	return err;
}

Boolean CheckTransmissionAllowed() {
	return !TrxMuted && TransmissionAllowedByState();
}

int TransmitSPLPacket(sat_packet_t *packet, int *avalFrames) {
	unsigned char packet_length = sizeof(sat_packet_t) - sizeof(packet->data) + packet->length; // only submit the actual data

	if (CheckTransmissionAllowed()) {
		for (int i = 0; i < 5; i++) {
			int err = IsisTrxvu_tcSendAX25DefClSign(0, (unsigned char *)packet, packet_length, (unsigned char *)avalFrames);

			if (*avalFrames == 0 || *avalFrames == 255) {
				vTaskDelay(100 / portTICK_RATE_MS);
			}
			else {
				if(err != E_NO_SS_ERR){
					logError(trxvu, __LINE__, err, "error sending packet");
				}
				return  err;
			}
		}
		return E_TIMEOUT;
	}
	else {
		taskYIELD();
		return E_CANT_TRANSMIT;
	}
}

int AssembleAndSendPacket(unsigned char *data, unsigned short data_length, char type, char subtype,unsigned int id) {
	sat_packet_t packet;
	int err = AssembleSPLPacket(data, data_length, type, subtype, id, &packet);
	if (err != E_NO_SS_ERR) {
		logError(trxvu, __LINE__, err, "error assembling packet");
		return err;
	}

	err = TransmitSPLPacket(&packet, NULL);
	if (err != E_NO_SS_ERR) {
		logError(trxvu, __LINE__, err, "error transmitting packet");
		return err;
	}
	return 0;
}



int taskDump(void * pvParameters) {
	int err = f_enterFS();
		if(err != 0){
			logError(tlm_management, __LINE__, err, "error entering FS");
			return err;
		}

	findData(CurrDumpArguments.dump_data.dump_type,
				CurrDumpArguments.dump_data.t_start,
				CurrDumpArguments.dump_data.t_end);
	f_releaseFS();
	DumpRunning = FALSE;
	vTaskDelete( NULL );
	return 0;
}

int StartDump(dump_arguments_t *arguments) {
	xTaskHandle taskDumpHandle = NULL;
	memcpy(&CurrDumpArguments, arguments, sizeof(dump_arguments_t));
	DumpRunning = TRUE;
	int err = xTaskCreate(taskDump, (const signed char*) "taskDump", 4096, NULL,
					configMAX_PRIORITIES - 2, &taskDumpHandle);
	return err;
}

void ResetGroundCommWDT() {
	Time time;
	Time_get_wrap(&time);
	time_unix unixtime = Time_convertTimeToEpoch(&time);
	FRAM_write((unsigned char*) &unixtime, LAST_COMM_TIME_ADDR, LAST_COMM_TIME_SIZE);
}

int BeaconLogic() {
	static unsigned int last_sent_time = 0; // last time the beacon was sent, in uptime seconds
	unsigned int interval;

	int err = GetBeaconInterval(&interval);
	if (err != 0) {
		logError(trxvu, __LINE__, err, "error getting beacon interval");
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
		logError(trxvu, __LINE__, err, "error sending beacon");
		return err;
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
			logError(trxvu, __LINE__, err, "error getting frame");
			return err;
		}

		err = ParseDataToSPLPacket(rxFrameCmd.rx_framedata, cmd);
		int sat_id = GetSatId(cmd);
		if (err == E_NO_SS_ERR && (sat_id == YCUBE_SAT_ID || sat_id == ALL_SAT_ID)) {
			return err;
		}

		err = IsisTrxvu_rcGetFrameCount(0, &RxCounter);
		if (err != E_NO_SS_ERR) {
			logError(trxvu, __LINE__, err, "error getting frame count");
			return err;
		}

	}

	return E_NO_COMMAND_FOUND;
}

int GetBeaconInterval(unsigned int *interval) {
    if (interval == NULL) {
		logError(trxvu, __LINE__, E_INPUT_POINTER_NULL, "interval pointer null");
        return E_INPUT_POINTER_NULL;
    }

    int err = FRAM_read((unsigned char*) &interval, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE);
    if (err != 0) {
		logError(trxvu, __LINE__, err, "error reading from FRAM");
        return err;
    }
    return 0;
}

int SetBeaconInterval(unsigned int *interval) {
    if (*interval < MIN_BEACON_INTERVAL || *interval > MAX_BEACON_INTERVAL) {
		logError(trxvu, __LINE__, E_PARAM_OUTOFBOUNDS, "given interval out of bounds");
        return E_PARAM_OUTOFBOUNDS;
    }

    int err = FRAM_write((unsigned char*) interval, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE);

    if (err != 0) {
		logError(trxvu, __LINE__, err, "error writing to FRAM");
        return err;
    }
    return 0;
}

int RestoreDefaultBeaconInterval() {
	unsigned int interval = DEFAULT_BEACON_INTERVAL_TIME;
    return SetBeaconInterval(&interval);
}

int SendAckPacket(ack_subtype_t acksubtype, sat_packet_t *cmd, unsigned char *data, unsigned short length) {
	int err = AssembleAndSendPacket(data, length, ack_type, acksubtype, cmd->ID);
	if (err != 0) {
		logError(trxvu, __LINE__, err, "error sending ACK packet");
		return err;
	}

	return 0;
}

int setMuteEndTime(time_unix endTime) {
	int err = FRAM_write((unsigned char*) &endTime, MUTE_END_TIME_ADDR, MUTE_END_TIME_SIZE);

	if (err != 0) {
		logError(trxvu, __LINE__, err, "error setting mute end time");
		return err;
	}

	return err;
}

int getMuteEndTime(time_unix *endTime) {
	int err = FRAM_read((unsigned char*) endTime, MUTE_END_TIME_ADDR, MUTE_END_TIME_SIZE);

	if (err != 0) {
		logError(trxvu, __LINE__, err, "error getting mute end time");
		return err;
	}

	return err;
}

int SetIdleState(ISIStrxvuIdleState state, time_unix duration) {
	//TODO: use duration to return to the default state "off" after a while
	return IsisTrxvu_tcSetIdlestate(0, state);
}

int muteTRXVU(time_unix duration) {
	Time time;
	Time_get_wrap(&time);
	time_unix unixtime = Time_convertTimeToEpoch(&time);
	unixtime += duration;
	int err = setMuteEndTime(unixtime);
	TrxMuted = TRUE;
	return err;
}

void UnMuteTRXVU() {
	Time time;
	Time_get_wrap(&time);
	time_unix unixtime = Time_convertTimeToEpoch(&time);
	setMuteEndTime(unixtime); // set the mute end time to now so that even if we get muted by a bit flip we will automatically unmute
	TrxMuted = FALSE;
}

Boolean CheckForMuteEnd() {
	Time time;
	Time_get_wrap(&time);
	time_unix unixtime = Time_convertTimeToEpoch(&time);
	time_unix endtime;
	getMuteEndTime(&endtime);
	return unixtime > endtime;
}


int AddDataToSendBuffer(unsigned char* data, int size) {
	if (dataInSendBuffer + size > MAX_COMMAND_DATA_LENGTH) {
		SendBuffer();
		dataInSendBuffer = 0; // reset buffer even if sending failed
	}

	memcpy(sendBuffer + dataInSendBuffer, data, size);
	dataInSendBuffer += size;
	return 0;
}

int SendBuffer() {
	if (dataInSendBuffer != 0) {
		int err = AssembleAndSendPacket(sendBuffer, dataInSendBuffer, dump_type, 0, CurrDumpArguments.cmd.ID);
		if (err != 0) {
			logError(trxvu, __LINE__, err, "error sending dump buffer");
			return err;
		}
		return -1;
	}

	return 0;
}

