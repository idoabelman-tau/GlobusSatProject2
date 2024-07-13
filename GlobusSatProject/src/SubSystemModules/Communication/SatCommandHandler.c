#include "SatCommandHandler.h"
#include "CommandDictionary.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int AssembleSPLPacket(unsigned char *data, unsigned short data_length, char type, char subtype,unsigned int id, sat_packet_t *cmd) {
	if (cmd == NULL) {
		return E_INPUT_POINTER_NULL;
	}

	cmd->ID = id;
	cmd->cmd_type = type;
	cmd->cmd_subtype = subtype;
	cmd->length = data_length;
	if (data_length > 0) {
		if (data == NULL) {
			return E_INPUT_POINTER_NULL;
		}
		memcpy(&cmd->data, data, data_length);
	}
	return E_NO_SS_ERR;
}

int ParseDataToSPLPacket(unsigned char * data, sat_packet_t *cmd) {
	if (cmd == NULL) {
		return E_INPUT_POINTER_NULL;
	}

	// treat the data buffer as a struct, checking that the length is right then copying to the new struct
	sat_packet_t * data_as_cmd = (sat_packet_t *) data;
	if (data_as_cmd->length > MAX_COMMAND_DATA_LENGTH) {
		return E_PARAM_OUTOFBOUNDS;
	}

	return AssembleSPLPacket(data_as_cmd->data, data_as_cmd->length, data_as_cmd->cmd_type, data_as_cmd->cmd_subtype, data_as_cmd->ID, cmd);
}

int ActUponCommand(sat_packet_t *cmd) {
	int cmd_res = 0;

	switch (cmd->cmd_type) {
	case trxvu_cmd_type:
		cmd_res = trxvu_command_router(cmd);
		break;
	case eps_cmd_type:
		cmd_res = eps_command_router(cmd);
		break;
	case telemetry_cmd_type:
		cmd_res = telemetry_command_router(cmd);
		break;
	case managment_cmd_type:
		cmd_res = trxvu_command_router(cmd);
		break;
	default:
		cmd_res = E_COMMAND_TYPE_NOT_FOUND;
	}

	if (cmd_res == E_NO_SS_ERR) {
		SendAckPacket(ACK_COMD_EXEC, cmd, NULL, 0);
	} else if (cmd_res == E_COMMAND_TYPE_NOT_FOUND) {
		SendAckPacket(ACK_UNKNOWN_TYPE, cmd, NULL, 0);
	}
	else {
		SendAckPacket(ACK_ERROR_MSG, cmd, (unsigned char *)&cmd_res, sizeof(char));
	}

	return 0;
}

int GetSatId(sat_packet_t *packet) {
	return (packet->ID & 0xFF); // get the LSB
}
