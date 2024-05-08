#include "SatCommandHandler.h"
#include <stdlib.h>
#include <string.h>

int AssembleSPLPacket(unsigned char *data, unsigned short data_length, char type, char subtype,unsigned int id, sat_packet_t *cmd) {
	cmd = malloc(sizeof(sat_packet_t));

	if (cmd == NULL) {
		return execution_error;
	}

	cmd->ID = id;
	cmd->cmd_type = type;
	cmd->cmd_subtype = subtype;
	cmd->length = data_length;
	memcpy(&cmd->data, data, data_length);
	return command_success;
}

int ParseDataToSPLPacket(unsigned char * data, sat_packet_t *cmd) {
	cmd = malloc(sizeof(sat_packet_t));

	if (cmd == NULL) {
		return execution_error;
	}

	// treat the data buffer as a struct, checking that the length is right then copying to the new struct
	sat_packet_t * data_as_cmd = (sat_packet_t *) data;
	if (data_as_cmd->length > MAX_COMMAND_DATA_LENGTH) {
		return index_out_of_bound;
	}

	return AssembleSPLPacket(data_as_cmd->data, data_as_cmd->length, data_as_cmd->cmd_type, data_as_cmd->cmd_subtype, data_as_cmd->ID, cmd);
}
