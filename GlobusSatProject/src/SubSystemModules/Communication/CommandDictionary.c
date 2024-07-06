#include "CommandDictionary.h"

// need to fix case numbering to match ground station expectations
/*int trxvu_command_router(sat_packet_t *cmd){
	switch(cmd->cmd_subtype){
		case(0):
			TRX_ping();
			break;
		case(1):
			setMuteEndTime(val); // how do we pass the correct time. is it in cmd->data?
			break;
	}
		case(2):
			getMuteEndTime();
			break;
		case(3):
			getTransponderEndTime();
			break;
		case(4):
			setTransponderRSSIinFRAM(val); // how do we pass the correct argument. is it in cmd->data?
			break;
		case(5):
			short hold= getTransponderRSSIFromFRAM(); // what do we do with the returnd value?
			break;
		case(6):
			InitTxModule();
			break;
		case(7):
			int hold = InitTrxvu(); // what do we do with the returnd value?
			break;
		case(8):
				checkTransponderFinish();
				break;
		case(9):
				int hold = CMD_SetBeaconInterval(sat_packet_t *cmd);
				break;
		case(10):
				int hold = TRX_Logic();
				break;
		case(11):
				int hold = SetRSSITransponder(rssiValue);
				break;
		case(12):
				int hold = turnOnTransponder();
				break;
		case(13):
				Boolean b = CheckDumpAbort();
				break;
		case(14):
				Boolean b =CheckTransmitionAllowed();
				break;
		case(15):
				int hold = TransmitSplPacket( packet,avalFrames);
				break;
		case(16):
				SendDumpAbortRequest();
				break;
		case(17):
				AbortDump(cmd);
				break;
		case(18):
				GetDelayedCommandBufferCount();
				break;
		case(19):
		FinishDump(sat_packet_t *cmd,unsigned char *buffer, ack_subtype_t acktype,
				unsigned char *err, unsigned int size);
				break;
		case(20):
				int hold =  BeaconLogic(Boolean forceTX);
				break;
		case(21):
				int hold = InitTrxvu();
				break;
		case(22):
				int hold = InitTrxvu();
				break;
		case(23):
				int hold = InitTrxvu();
				break;
		case(24):
				int hold = InitTrxvu();
				break;
		case(25):
				int hold = InitTrxvu();
				break;
}*/
