
#include "TRXVU_Commands.h"

int CMD_Ping(sat_packet_t *cmd) {
	return SendAckPacket(ACK_PING, &cmd, NULL, 0);
}

