
#include "TRXVU_Commands.h"

int CMD_Ping(sat_packet_t *cmd) {
	return SendAckPacket(ACK_PING, &cmd, NULL, 0);
}

int CMD_StartDump(sat_packet_t *cmd) {
	dump_arguments_t dumpArguments;
	memcpy(&(dumpArguments.cmd), cmd, sizeof(sat_packet_t));
	memcpy(&(dumpArguments.dump_data), cmd->data, sizeof(dump_data));
	return StartDump(&dumpArguments) ;
}

