#include "TelemetryTestingDemo.h"

Boolean TestTelemetryCollection() {
	TelemetryCollectorLogic();
	return TRUE;
}

Boolean TestTelemetryRetrievel(){
	int selection;
	int bounds[2];
	printf("enter time stamp of element to be found\r\n");
	UTIL_DbguGetIntegerMinMax(&selection, 0, 90000);
	bounds[0] = selection;
	bounds[1] = selection;
	findData(tlm_eps,bounds);
	return TRUE;
}

Boolean TestTelemetryInterval(){
		int bounds[2];
		printf("enter time stamp From\n\r");
		UTIL_DbguGetIntegerMinMax(&bounds[0], 0, 90000);
		printf("enter time stamp To\n\r");
		UTIL_DbguGetIntegerMinMax(&bounds[1], 0, 90000);
		findData(tlm_eps,bounds);
		return TRUE;
}

Boolean selectAndExecuteTelemetryTest() {
	int selection = 0;

	printf( "\n\r Select the test to perform: \n\r");
	printf("\t 1) Test telemetry collection \n\r");
	printf("\t 2) find telemetry data \n\r");
	printf("\t 3) find telemetry data by interval \n\r");
	printf("\t 3) Go back \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 5) == 0);

	switch(selection)
	{
		case 1:
			if(!TestTelemetryCollection()) {
				printf("TestTelemetryCollection failed");
			}
			break;
		case 2:

			if(!TestTelemetryRetrievel()) {
					printf("TestTelemetryRetrievel failed");
			}
				break;
		case 3:
			if(!TestTelemetryInterval()) {
				printf("TestTelemetryRetrievel by interval failed");
			}
			break;
		case 4:
			return FALSE;

		default:
			break;
	}

	return TRUE;
}

Boolean MainTelemetryTestBench() {
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecuteTelemetryTest();

		if(offerMoreTests == FALSE)
		{
			break;
		}
	}
	return TRUE;
}
