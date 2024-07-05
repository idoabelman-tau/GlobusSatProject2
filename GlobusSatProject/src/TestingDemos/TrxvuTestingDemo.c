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
	if (err != E_NO_COMMAND_FOUND) {
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
	if (err != E_NO_COMMAND_FOUND) {
		printf("GetOnlineCommand failed or got unexpected result: %d\n", err);
		return FALSE;
	} else {
		printf("GetOnlineCommand found no command as expected\n");
	}

	printf("Testing getting command with one meant for us available\n");
	printf("Send command with ID equal %d and input any number\n", YCUBE_SAT_ID);
	UTIL_DbguGetInteger(&readval);
	err = GetOnlineCommand(&cmd);
	if (err != E_NO_SS_ERR) {
		printf("GetOnlineCommand failed or got unexpected result: %d\n", err);
		return FALSE;
	} else {
		printf("GetOnlineCommand found command as expected\n");
		PrintPacket(&cmd);
	}

	return TRUE;
}

Boolean TestTrxLogic() {
	printf("Testing full TRX logic\n");
	printf("Send command and input any number\n", YCUBE_SAT_ID);
	int err = TRX_Logic();
	if (err != 0) {
		printf("Error in TRX_Logic");
	}

	return TRUE;
}

Boolean selectAndExecuteTRXVUTest() {
	int selection = 0;

		printf( "\n\r Select the test to perform: \n\r");
		printf("\t 1) Send beacon \n\r");
		printf("\t 2) Beacon logic \n\r");
		printf("\t 3) Get online command \n\r");
		printf("\t 4) Full TRX logic \n\r");
		printf("\t 5) Go back \n\r");

		while(UTIL_DbguGetIntegerMinMax(&selection, 1, 2) == 0);

		switch(selection)
		{
			case 1:
				if(!TestSendBeacon()) {
					printf("TestSendBeacon failed");
				}
				break;
			case 2:
				if(!TestBeaconLogic()) {
					printf("TestBeaconLogic failed");
				}
				break;
			case 3:
				if(!TestGetOnlineCommand()) {
					printf("TestGetOnlineCommand failed");
				}
				break;
			case 4:
				if(!TestTrxLogic()) {
					printf("TestTrxLogic failed");
				}
				break;

			case 5:
				return FALSE;

			default:
				break;
		}

	    return TRUE;
}

Boolean MainTrxvuTestBench() {

	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecuteTRXVUTest();

		if(offerMoreTests == FALSE)
		{
			break;
		}
	}
	return TRUE;
}
