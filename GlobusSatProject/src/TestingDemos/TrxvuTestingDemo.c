#include "TrxvuTestingDemo.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

int PrintPacket(sat_packet_t *cmd) {
	printf("Packet details:\n");
	printf("ID: %d\n", cmd->ID);
	printf("Command type: %d\n", (int)cmd->cmd_type);
	printf("Command subtype: %d\n", (int)cmd->cmd_subtype);
	printf("Command data length: %d\n", cmd->length);
	printf("Command data: ");
	for (int i = 0; i < cmd->length; i++) {
		printf("%02x ", cmd->data[i]);
	}
	printf("\n");
	return 0;
}

Boolean TestSendBeacon() {
	printf("testing sending beacon\n");
	return SendBeacon() == 0;
}

Boolean TestBeaconLogic() {
	printf("Updating beacon delay to 10 seconds \n");
	unsigned int interval = 10;
	if (SetBeaconInterval(&interval) != 0) {
		printf("Failed to update beacon delay \n");
		return FALSE;
	} else {
		printf("Updated beacon delay to 10 seconds\n");
	}

	printf("Sleeping 10s\n");
	vTaskDelay(10000 / portTICK_RATE_MS);
	printf("testing beacon logic\n");

	int err = BeaconLogic();
	if (err != 0) {
		printf("Beacon logic failed\n");
	} else {
		printf("Beacon logic succeeded, should see a beacon \n");
	}

	printf("testing beacon logic with no delay\n");
	err = BeaconLogic();
	if (err != 0) {
		printf("Beacon logic failed\n");
		return FALSE;
	} else {
		printf("Beacon logic succeeded, should not see another beacon \n");
	}

	printf("Sleeping 10s\n");
	vTaskDelay(10000 / portTICK_RATE_MS);
	printf("testing beacon logic with delay\n");

	err = BeaconLogic();
	if (err != 0) {
		printf("Beacon logic failed\n");
		return FALSE;
	} else {
		printf("Beacon logic succeeded, should see a beacon \n");
	}

	return TRUE;
}

Boolean TestGetOnlineCommand() {
	sat_packet_t cmd = {0};

	printf("Testing getting command with none available\n");
	int err = GetOnlineCommand(&cmd);
	if (err != no_command_found) {
		printf("GetOnlineCommand failed or got unexpected result: %d\n", err);
		return FALSE;
	} else {
		printf("GetOnlineCommand found no command as expected\n");
	}

	printf("Testing getting command with one not meant for us available\n");
	printf("Send command with ID not equal %d and input any number\n", YCUBE_SAT_ID);
	int readval;
	UTIL_DbguGetInteger(&readval);
	err = GetOnlineCommand(&cmd);
	if (err != no_command_found) {
		printf("GetOnlineCommand failed or got unexpected result: %d\n", err);
		return FALSE;
	} else {
		printf("GetOnlineCommand found no command as expected\n");
	}

	printf("Testing getting command with one meant for us available\n");
	printf("Send command with ID equal %d and input any number\n", YCUBE_SAT_ID);
	UTIL_DbguGetInteger(&readval);
	err = GetOnlineCommand(&cmd);
	if (err != command_found) {
		printf("GetOnlineCommand failed or got unexpected result: %d\n", err);
		return FALSE;
	} else {
		printf("GetOnlineCommand found command as expected\n");
		PrintPacket(&cmd);
	}

	return TRUE;
}

Boolean MainTrxvuTestBench() {
    Boolean send_beacon_success = TestSendBeacon();
    Boolean beacon_logic_success = TestBeaconLogic();
    Boolean online_command_success = TestGetOnlineCommand();
    printf("TestSendBeacon: %s\n", send_beacon_success ? "SUCCESS" : "FAIL");
    printf("TestBeaconLogic: %s\n", beacon_logic_success ? "SUCCESS" : "FAIL");
    printf("TestGetOnlineCommand: %s\n", online_command_success ? "SUCCESS" : "FAIL");
    return TRUE;
}
