#include "MainTest.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <stdio.h>

void PrintTime(Time time) {
	printf("%d/%d/%d %d:%d:%d\n", time.year, time.month, time.date, time.hours, time.minutes, time.seconds);
}

Boolean DumpTelemetryTest() {
	SetUseStub(FALSE);
	Time time;
	Time_get(&time);
	printf("Test start: ");
	PrintTime(time);

	time_unix start_time = Time_convertTimeToEpoch(time);
	time_unix curr_time = start_time;
	unsigned int cycles = 0;
	while (curr_time - start_time < 60*5) { // run with no dump for 5 minutes
		EPS_Conditioning();
		TRX_Logic();
		TelemetryCollectorLogic();
		Maintenance();

		Time_get(&time);
		curr_time = Time_convertTimeToEpoch(time);
		cycles++;
	}

	printf("Num cycles with no dump: %d\n", cycles);
	printf("Average cycle length no dump: %f\n", (double)(curr_time - start_time) / (double)cycles);

	printf("Start dump time: ");
	PrintTime(time);
	time_unix start_dump_time = curr_time;
	dump_arguments_t args = {0};
	args.t_start = 2000;
	SetIdleState(trxvu_idle_state_on, 0);
	StartDump(args);
	cycles = 0;

	while (DumpRunning) {
		EPS_Conditioning();
		TRX_Logic();
		TelemetryCollectorLogic();
		Maintenance();

		cycles++;
	}

	SetIdleState(trxvu_idle_state_off, 0);

	Time_get(&time);
	printf("End dump time: ");
	PrintTime(time);
	curr_time = Time_convertTimeToEpoch(time);
	printf("Num cycles with dump: %d\n", cycles);
	printf("Average cycle length with dump: %f\n", (double)(curr_time - start_dump_time) / (double)cycles);

	time_unix start_final_loop_time = curr_time;
	cycles = 0;
	while (curr_time - start_final_loop_time < 60*5) { // run with no dump for 5 minutes
		EPS_Conditioning();
		TRX_Logic();
		TelemetryCollectorLogic();
		Maintenance();

		Time_get(&time);
		curr_time = Time_convertTimeToEpoch(time);
		cycles++;
	}

	printf("End test time: ");
	PrintTime(time);
	printf("Num cycles with no dump (second loop): %d\n", cycles);
	printf("Average cycle length no dump (second loop): %f\n", (double)(curr_time - start_final_loop_time) / (double)cycles);

	return TRUE;
}

Boolean DummyVoltageTest() {
	SetUseStub(TRUE);
	srand(0);
	Time time;
	Time_get(&time);
	printf("Test start: ");
	PrintTime(time);

	SetVoltage(7500);
	filtered_voltage = 7500;
	voltage_t base_voltage = 7500;
	int voltage_noise_level = 10;
	int voltage_noise = 0;

	dump_arguments_t args = {0};
	args.t_start = 2000;
	SetIdleState(trxvu_idle_state_on, 0);
	StartDump(args);
	unsigned int cycles = 0;

	while (DumpRunning) {
		EPS_Conditioning();
		TRX_Logic();
		TelemetryCollectorLogic();
		Maintenance();

		cycles++;
		base_voltage -= 1;
		voltage_noise = rand() % (voltage_noise_level * 2 + 1) - voltage_noise_level; // number between -voltage_noise_level and voltage_noise_level
		SetVoltage(base_voltage + voltage_noise);
	}

	SetIdleState(trxvu_idle_state_off, 0);
}

Boolean selectAndExecuteTest()
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select the device to be tested to perform: \n\r");
	printf("\t 1) EPS test \n\r");
	printf("\t 2) TRXVU Test \n\r");
	printf("\t 3) Full System Dump Telemetry Test \n\r");
	printf("\t 4) Full System Dummy Voltage Test \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 2) == 0);

	switch(selection)
	{
		case 1:
			offerMoreTests = MainEpsTestBench();
			break;
		case 2:
			offerMoreTests = MainTrxvuTestBench();
			break;
		case 3:
			offerMoreTests = DumpTelemetryTest();
			break;
		case 4:
			offerMoreTests = DummyVoltageTest();
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
