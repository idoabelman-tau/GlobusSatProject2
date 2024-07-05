#include "CommandDictionary.h"

int trxvu_command_router(sat_packet_t *cmd) {
	switch (cmd->cmd_subtype) {
		case PING:
			return CMD_Ping(cmd);
			break;
		default:
			return E_COMMAND_TYPE_NOT_FOUND;
	}
}

int eps_command_router(sat_packet_t *cmd) {
	switch (cmd->cmd_subtype) {
		default:
			return E_COMMAND_TYPE_NOT_FOUND;
	}
}

int telemetry_command_router(sat_packet_t *cmd) {
	switch (cmd->cmd_subtype) {
		default:
			return E_COMMAND_TYPE_NOT_FOUND;
	}
}

int management_command_router(sat_packet_t *cmd) {
	switch (cmd->cmd_subtype) {
		default:
			return E_COMMAND_TYPE_NOT_FOUND;
	}
}

