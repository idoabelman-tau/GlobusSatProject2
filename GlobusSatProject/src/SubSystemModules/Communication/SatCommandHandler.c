#include "SatCommandHandler.h"




int AssembleCommand(unsigned char *data, unsigned short data_length, char type, char subtype,unsigned int id, sat_packet_t *cmd){
	cmd->cmd_type = type;
	cmd->cmd_subtype = subtype;
	cmd->ID = id;
	for(int i = 0 ; i< data_length ; i++){
		cmd->data[i] = data[i];
	}
	cmd->length = data_length;
	return command_succsess;
}
