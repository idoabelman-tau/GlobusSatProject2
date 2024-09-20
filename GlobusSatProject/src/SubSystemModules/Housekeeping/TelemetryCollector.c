
#include "TelemetryCollector.h"
WOD_Telemetry_t dummy_wod = {0};



//TODO current tests will be on EPS only and other functions will have a return statement after call.

static xTimerHandle timerArray[5];



void GetCurrentWODTelemetry(WOD_Telemetry_t *wod){
	if (NULL == wod){
		return;
	}

	memset(wod,0,sizeof(*wod));
	int err = 0;


	err = f_enterFS(); // need to enter FS for some filesystem related WOD telemetry
	if(err != 0){
		logError(telemetry_collector, __LINE__, err, "error entering FS");
		return;
	}

	FN_SPACE space = { 0 };
	int drivenum = f_getdrive();

	// get the free space of the SD card
	err = f_getfreespace(drivenum, &space);

	f_releaseFS();

	if (err == F_NO_ERROR){
		wod->free_memory = space.free;
		wod->corrupt_bytes = space.bad;
	}else {
		logError(telemetry_collector, __LINE__, err, "error getting free space");
	}

	time_unix current_time = 0;
	Time_getUnixEpoch((unsigned int *)&current_time);
	wod->sat_time = current_time;

#ifdef ISISEPS
	imepsv2_piu__gethousekeepingeng__from_t hk_tlm;
	imepsv2_piu__gethousekeepingengincdb__from_t hk_tlm_cdb;

	err =  imepsv2_piu__gethousekeepingengincdb(EPS_I2C_BUS_INDEX, &hk_tlm_cdb);
	err += imepsv2_piu__gethousekeepingeng(EPS_I2C_BUS_INDEX, &hk_tlm);

	if(err == 0){

		wod->electric_current = hk_tlm.fields.vip_obc00.fields.current;
		wod->vbat = hk_tlm_cdb.fields.dist_input.fields.volt;
		wod->current_3V3 = hk_tlm.fields.vip_obc05.fields.current;
		wod->current_5V = hk_tlm.fields.vip_obc01.fields.current;
		wod->volt_3V3 = hk_tlm.fields.vip_obc05.fields.volt;
		wod->volt_5V = hk_tlm.fields.vip_obc01.fields.volt;
		wod->mcu_temp = hk_tlm.fields.temp;
		wod->bat_temp = hk_tlm.fields.temp3;
		wod->charging_power = hk_tlm_cdb.fields.batt_input.fields.volt;
		wod->consumed_power = hk_tlm_cdb.fields.dist_input.fields.power;
		// set all solar panels temp values
		uint8_t status;
		IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_0,&wod->solar_panels[0],&status);
		IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_1,&wod->solar_panels[1],&status);
		IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_2,&wod->solar_panels[2],&status);
		IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_3,&wod->solar_panels[3],&status);
		IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_4,&wod->solar_panels[4],&status);
		IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_5,&wod->solar_panels[5],&status);
	}else {
		logError(telemetry_collector, __LINE__, err, "error getting eps telemetry");
	}
#endif
#ifdef GOMEPS
	gom_eps_hk_t gom_hk_tlm;
	err = GomEpsGetHkData_general(0, &gom_hk_tlm);
	if(err == 0){

		wod->electric_current = gom_hk_tlm.fields.curout[0];
		wod->vbat = gom_hk_tlm.fields.vbatt;
		wod->current_3V3 = gom_hk_tlm.fields.curout[5];
		wod->current_5V = gom_hk_tlm.fields.curout[1];
		wod->volt_3V3 = 0; // unset
		wod->volt_5V = 0; // unset
		wod->mcu_temp = gom_hk_tlm.fields.temp[0]; // TEMP1 sensor
		wod->bat_temp = gom_hk_tlm.fields.temp[4]; // BATT0 sensor
		wod->charging_power = gom_hk_tlm.fields.curin[0]; // input current in mA
		wod->consumed_power = gom_hk_tlm.fields.vbatt * gom_hk_tlm.fields.curout[0] / 1000; // computed power based on battery voltage and primary current
		// set all solar panels temp values
		uint8_t status;
		IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_0,&wod->solar_panels[0],&status);
		IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_1,&wod->solar_panels[1],&status);
		IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_2,&wod->solar_panels[2],&status);
		IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_3,&wod->solar_panels[3],&status);
		IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_4,&wod->solar_panels[4],&status);
		IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_5,&wod->solar_panels[5],&status);
	}else {
		logError(telemetry_collector, __LINE__, err, "error getting eps telemetry");
	}
#endif

	int reset=0;
	err = FRAM_read((unsigned char *)&reset,NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE);
	wod->number_of_resets = reset;
	err = FRAM_read((unsigned char *)&reset,NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_SIZE);
	wod->num_of_cmd_resets = reset;

	wod->sat_uptime = Time_getUptimeSeconds();

	// get ADC channels vlaues (include the photo diodes mV values)
	unsigned short adcSamples[8];

	ADC_SingleShot( adcSamples );

	for(int i=0; i <= 4; i++ )
	{
		wod->photo_diodes[i] = ADC_ConvertRaw10bitToMillivolt( adcSamples[i] ); // convert to mV data
		////printf("PD%d : %u mV\n\r", i, wod->photo_diodes[i]);
	}


}

void TelemetrySaveEPS(xTimerHandle pxTimer){
	/*
	 imepsv2_piu__gethousekeepingeng__from_t epsTlmStruct;
	 int err = imepsv2_piu__gethousekeepingeng(0,&epsTlmStruct);
	*/

	unsigned char * data_out_ptr;
	int err;

#ifdef ISISEPS
    imepsv2_piu__gethousekeepingeng__from_t data_out;
    int err = imepsv2_piu__gethousekeepingeng(0,&data_out);
    data_out_ptr = (unsigned char *)&data_out;
#endif
#ifdef GOMEPS
    gom_eps_hk_t data_out; // TODO:make sure this is the correct data structure.
    err = GomEpsGetHkData_general(0,&data_out );
    data_out_ptr = (unsigned char *)&data_out;
#endif

	if(err < 0){
		logError(telemetry_collector, __LINE__, err, "error getting eps telemetry");
		return;
	}

	WriteData(tlm_eps,data_out_ptr);

	return;
}


void TelemetrySaveTRXVU(xTimerHandle pxTimer){
	ISIStrxvuTxTelemetry TxTlmStruct;
	ISIStrxvuRxTelemetry RxTlmStruct;

	int err = IsisTrxvu_tcGetTelemetryAll(ISIS_TRXVU_I2C_BUS_INDEX, &TxTlmStruct);
	if(err){
		logError(telemetry_collector, __LINE__, err, "error getting TX telemetry");
		return;
	}

	WriteData(tlm_tx,TxTlmStruct.raw);

	err = IsisTrxvu_rcGetTelemetryAll(ISIS_TRXVU_I2C_BUS_INDEX,&RxTlmStruct);
	if(err){
		logError(telemetry_collector, __LINE__, err, "error getting RX telemetry");
		return;
	}

	WriteData(tlm_rx,RxTlmStruct.raw);

}

void TelemetrySaveANT(xTimerHandle pxTimer){
	ISISantsTelemetry ant_tlmA, ant_tlmB;

	int err = IsisAntS_getAlltelemetry(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideA,
			&ant_tlmA);
	if(err){
		logError(telemetry_collector, __LINE__, err, "error getting antenna telemetry");
		return;
	}
	WriteData(tlm_antenna,(unsigned char *) &ant_tlmA);

	err = IsisAntS_getAlltelemetry(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideB,
			&ant_tlmB);
	if(err){
		logError(telemetry_collector, __LINE__, err, "error getting antenna telemetry");
		return;
	}
	WriteData(tlm_antenna,(unsigned char *) &ant_tlmB);

}

void TelemetrySaveSOLAR(xTimerHandle pxTimer){
	SolarPack sp[NUMBER_OF_SOLAR_PANELS];

	for(int antNum = 0 ; antNum<NUMBER_OF_SOLAR_PANELS ; antNum++){
		sp[antNum].parsed.state = IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_0,&(sp[antNum].parsed.temp),&(sp[antNum].parsed.status));
	}
	WriteData(tlm_solar,(unsigned char *) sp);

	return;

}



void TelemetrySaveWOD(xTimerHandle pxTimer){
	WOD_Telemetry_t wod;

	GetCurrentWODTelemetry(&wod);

	WriteData(tlm_wod, (unsigned char *)&wod);

	return;
}

int updateSaveTime(tlm_type_t tlm, int sec){
	int err = FRAM_write( (unsigned char *) &sec,  TLM_SAVE_PERIOD_START_ADDR + 4*tlm, 4);
	if(err){
		logError(telemetry_collector, __LINE__, err, "failed to update save time in FRAM");
		return err;
	}

	err = xTimerChangePeriod(timerArray[tlm],MINUTES_TO_TICKS(sec),0);
	if (err == pdFAIL) {
		logError(telemetry_collector, __LINE__, E_TIMER_UPDATE_FAIL, "failed to update save timer");
		return E_TIMER_UPDATE_FAIL;
	}
	return 0;

}

void TelemetryEnterFS(xTimerHandle pxTimer) {
	int err = f_enterFS();
	if(err != 0){
		logError(tlm_management, __LINE__, err, "error entering FS");
		return;
	}
}

void TelemetryCollectorLogic(){
	int sec;
	int err = FRAM_read( (unsigned char *)&sec,  EPS_SAVE_TLM_PERIOD_ADDR, 4);
	if(err){
		logError(telemetry_collector, __LINE__, err, "failed to update read time from FRAM");
		return;
	}
	timerArray[0] = xTimerCreate((const signed char *)"EPS_TIMER",
					MINUTES_TO_TICKS(sec),
					pdTRUE,
					(void*) 0,
					TelemetrySaveEPS);

	err = FRAM_read( (unsigned char *)&sec,  TRXVU_SAVE_TLM_PERIOD_ADDR, 4);
	if(err){
		logError(telemetry_collector, __LINE__, err, "failed to update read time from FRAM");
		return;
	}
	timerArray[1] = xTimerCreate((const signed char *)"TRXVU_TIMER",
					MINUTES_TO_TICKS(sec),
					pdTRUE,
					(void*) 1,
					TelemetrySaveTRXVU);

	err = FRAM_read( (unsigned char *)&sec,  ANT_SAVE_TLM_PERIOD_ADDR, 4);
	if(err){
		logError(telemetry_collector, __LINE__, err, "failed to update read time from FRAM");
		return;
	}
	timerArray[2] = xTimerCreate((const signed char *)"ANT_TIMER",
					MINUTES_TO_TICKS(sec),
					pdTRUE,
					(void*) 2,
					TelemetrySaveANT);

	err = FRAM_read( (unsigned char *)&sec,  SOLAR_SAVE_TLM_PERIOD_ADDR, 4);
	if(err){
		logError(telemetry_collector, __LINE__, err, "failed to update read time from FRAM");
		return;
	}
	timerArray[3] = xTimerCreate((const signed char *)"SOLAR_TIMER",
						MINUTES_TO_TICKS(sec),
						pdTRUE,
						(void*) 3,
						TelemetrySaveSOLAR);

	err = FRAM_read( (unsigned char *)&sec,  WOD_SAVE_TLM_PERIOD_ADDR, 4);
	if(err){
		logError(telemetry_collector, __LINE__, err, "failed to update read time from FRAM");
		return;
	}
	timerArray[4] = xTimerCreate((const signed char *)"WOD_TIMER",
							MINUTES_TO_TICKS(sec),
							pdTRUE,
							(void*) 4,
							TelemetrySaveWOD);

	for(int i = 0 ; i < 5 ; i++){
		if(timerArray[i] == NULL){
			logError(telemetry_collector, __LINE__, err, "failed to initialize timer");
		}
	}
	for(int i = 0 ; i < 5 ; i++){
		if(xTimerStart(timerArray[i],0) != pdPASS){
			logError(telemetry_collector, __LINE__, err, "failed to start timer");
		}
	}

	xTimerHandle enterTimer = xTimerCreate((const signed char *)"ENTER_TIMER",
			1,
			pdFALSE,
			(void*) 5,
			TelemetryEnterFS);
	if(xTimerStart(enterTimer,0) != pdPASS){
		logError(telemetry_collector, __LINE__, err, "failed to start timer");
	}
}

