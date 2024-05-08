#include "TRXVU.h"
#include "SubSystemModules/Housekeeping/TelemetryCollector.h"
#include "SatCommandHandler.h"

#define RECIEVER_I2C_ADDRESS 0x60
#define TRANSMITTER_I2C_ADDRESS 0x61

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

	TRX_BIT_RATE_STRUCT = trxvu_bitrate_1200;  //TODO: set correct bit

	// initialize
	TRX_ERR_FLAG = IsisTrxvu_initialize(&TRX_I2C_ADDR_STRUCT,&TRX_FRAME_LENGTH_STRUCT,&TRX_BIT_RATE_STRUCT,1);

	if(TRX_ERR_FLAG != E_NO_SS_ERR && TRX_ERR_FLAG != E_IS_INITIALIZED){
		//TODO: log error
		return  TRX_ERR_FLAG;
	}

	return E_NO_SS_ERR;
}

int TRX_Logic() {
	sat_packet_t cmd;
	int err = GetOnlineCommand(&cmd);

	/*if(err == no_command_found) {
		err = GetDelayedCommand(sat_packet_t *cmd);
	}*/ // delayed commands not currently implemented

	if (err == command_found) { // a command was found either in online or delayed buffer
		err = ActUponCommand(&cmd);
	}
	else if (err != no_command_found) {
		// TODO: log error
		printf("error getting command\n");
	}

	BeaconLogic();

	return err;
}

int TransmitSPLPacket(sat_packet_t *packet, int *avalFrames) {
	unsigned char packet_length = sizeof(sat_packet_t) - sizeof(packet->data) + packet->length; // only submit the actual data
	int err = IsisTrxvu_tcSendAX25DefClSign(TRANSMITTER_I2C_ADDRESS, (unsigned char *)packet, packet_length, &avalFrames);

	if(err != E_NO_SS_ERR){
		printf("error sending packet\n");
		//TODO: log error
		return  err;
	}

	return err;
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
		last_sent_time = Time_getUptimeSeconds();
		return SendBeacon();
	}

	return 0;
}

int SendBeacon() {

	WOD_Telemetry_t wod;
	GetCurrentWODTelemetry(&wod);

	sat_packet_t beacon;

	int err = AssembleSPLPacket((unsigned char *)&wod, sizeof(wod), trxvu_cmd_type, BEACON_SUBTYPE, YCUBE_SAT_ID, &beacon);
	if (err != command_success) {
		// TODO: log error
		return -1;
	}

	err = TransmitSPLPacket(&beacon, NULL);
	if (err != E_NO_SS_ERR) {
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
		return execution_error;
	}

	while(RxCounter > 0)
	{
		int err = IsisTrxvu_rcGetCommandFrame(0, &rxFrameCmd);
		if (err != E_NO_SS_ERR) {
			// TODO: log error
			return execution_error;
		}

		err = ParseDataToSPLPacket(&(rxFrameCmd.rx_framedata), cmd);
		if (err == command_success && (cmd->ID == YCUBE_SAT_ID || cmd->ID == ALL_SAT_ID)) {
			return command_found;
		}

		err = IsisTrxvu_rcGetFrameCount(0, &RxCounter);
		if (err != E_NO_SS_ERR) {
			// TODO: log error
			return execution_error;
		}

	}

	return no_command_found;
}
