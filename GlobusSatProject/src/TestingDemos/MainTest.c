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

	time_unix start_time = Time_convertTimeToEpoch(&time);
	time_unix curr_time = start_time;
	unsigned int cycles = 0;
	while (curr_time - start_time < 60) { // run with no dump for 5 minutes
		EPS_Conditioning();
		//printf("state: %d\n", GetSystemState());
		TRX_Logic();
		TelemetryCollectorLogic();
		Maintenance();

		Time_get(&time);
		curr_time = Time_convertTimeToEpoch(&time);
		cycles++;
	}

	printf("Num cycles with no dump: %d\n", cycles);
	printf("Average cycle length no dump: %f\n", (double)(curr_time - start_time) / (double)cycles);

	printf("Start dump time: ");
	PrintTime(time);
	time_unix start_dump_time = curr_time;
	dump_arguments_t args = {0};
	args.dump_data.t_start = 500;
	SetIdleState(trxvu_idle_state_on, 0);
	StartDump(&args);
	unsigned char data[MAX_COMMAND_DATA_LENGTH] = {0};
	unsigned int *dump_index = (unsigned int *)data; // point to the first 4 bytes as integer
	cycles = 0;

	// dummy dump sending packets based on t_start
	//printf("entering loop during dump time: ");
	while (DumpRunning) {
		//printf("cycles while dump is running: %d\r\n", i);
		EPS_Conditioning();
		//printf("state: %d\n", GetSystemState());
		TRX_Logic();
		TelemetryCollectorLogic();
		Maintenance();

		cycles++;
	}

	SetIdleState(trxvu_idle_state_off, 0);

	Time_get(&time);
	printf("End dump time: ");
	PrintTime(time);
	curr_time = Time_convertTimeToEpoch(&time);
	printf("Num cycles with dump: %d\n", cycles);
	printf("Average cycle length with dump: %f\n", (double)(curr_time - start_dump_time) / (double)cycles);

	time_unix start_final_loop_time = curr_time;
	cycles = 0;
	while (curr_time - start_final_loop_time < 10) { // run with no dump for 5 minutes
		EPS_Conditioning();
		TRX_Logic();
		TelemetryCollectorLogic();
		Maintenance();

		Time_get(&time);
		curr_time = Time_convertTimeToEpoch(&time);
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


	for (int i = 0; i < 1200; i++) {
		Time_get(&time);
		printf("Time: ");
		PrintTime(time);
		printf("State: %d\n\r", GetSystemState());
		voltage_t vbat = 0;
		GetBatteryVoltage(&vbat);
		printf("Raw voltage: %d\n\r", vbat);
		printf("Filtered voltage: %d\n\r", filtered_voltage);

		EPS_Conditioning();
		TRX_Logic();
		TelemetryCollectorLogic();
		Maintenance();

		base_voltage -= 1;
		voltage_noise = rand() % (voltage_noise_level * 2 + 1) - voltage_noise_level; // number between -voltage_noise_level and voltage_noise_level
		SetVoltage(base_voltage + voltage_noise);
	}

	for (int i = 0; i < 1200; i++) {
		Time_get(&time);
		printf("Time: ");
		PrintTime(time);
		printf("State: %d\n\r", GetSystemState());
		voltage_t vbat = 0;
		GetBatteryVoltage(&vbat);
		printf("Raw voltage: %d\n\r", vbat);
		printf("Filtered voltage: %d\n\r", filtered_voltage);

		EPS_Conditioning();
		TRX_Logic();
		TelemetryCollectorLogic();
		Maintenance();

		base_voltage += 1;
		voltage_noise = rand() % (voltage_noise_level * 2 + 1) - voltage_noise_level; // number between -voltage_noise_level and voltage_noise_level
		SetVoltage(base_voltage + voltage_noise);
	}

	Time_get(&time);
	printf("Time: ");
	PrintTime(time);
	printf("State: %d\n\r", GetSystemState());
	voltage_t vbat = 0;
	GetBatteryVoltage(&vbat);
	printf("Raw voltage: %d\n\r", vbat);
	printf("Filtered voltage: %d\n\r", filtered_voltage);
	return TRUE;
}

Boolean selectAndExecuteTest()
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select the device to be tested to perform: \n\r");
	printf("\t 1) EPS test \n\r");
	printf("\t 2) TRXVU Test \n\r");
	printf("\t 3) Telemetry Test \n\r");
	printf("\t 4) Full System Dump Telemetry Test \n\r");
	printf("\t 5) Full System Dummy Voltage Test \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 5) == 0);

	switch(selection)
	{
		case 1:
			offerMoreTests = MainEpsTestBench();
			break;
		case 2:
			offerMoreTests = MainTrxvuTestBench();
			break;
		case 3:
			offerMoreTests = MainTelemetryTestBench();
			break;
		case 4:
			offerMoreTests = DumpTelemetryTest();
			break;
		case 5:
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
