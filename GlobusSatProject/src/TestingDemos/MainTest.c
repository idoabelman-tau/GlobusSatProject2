#include "MainTest.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <stdio.h>

Boolean selectAndExecuteTest()
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select the device to be tested to perform: \n\r");
	printf("\t 1) EPS test \n\r");
	printf("\t 2) TRXVU Test \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 2) == 0);

	switch(selection)
	{
		case 1:
			offerMoreTests = MainEpsTestBench();
			break;
		case 2:
			offerMoreTests = MainTrxvuTestBench();
			break;

		default:
			break;
	}

	return offerMoreTests;
}

void taskTesting()
{
	Boolean offerMoreTests = FALSE;

	WDT_startWatchdogKickTask(10 / portTICK_RATE_MS, FALSE);

	while(1)
	{
		offerMoreTests = selectAndExecuteTest();

		if(offerMoreTests == FALSE)
		{
			break;
		}
	}

	while(1) {
		vTaskDelay(500);
	}

}
