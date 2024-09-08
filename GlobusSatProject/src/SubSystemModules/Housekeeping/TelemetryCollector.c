
#include "TelemetryCollector.h"
WOD_Telemetry_t dummy_wod = {0};



//TODO current tests will be on EPS only and other functions will have a return statement after call.

static xTimerHandle timerArray[5];



void GetCurrentWODTelemetry(xTimerHandle pxTimer){
	return;
	// dummy for testing
	imepsv2_piu__gethousekeepingengincdb__from_t wodTlmStruct;
	int err = imepsv2_piu__gethousekeepingengincdb(EPS_I2C_BUS_INDEX,&wodTlmStruct);
	if(err){
		//TODO: log error
		printf("failed in retrieving WOD house keeping data. (TLM_collector.c / collect_WOD)");
		return;
	}
	//wod = NULL;
	//wod->bat_temp = wodTlmStruct->fields->temp3; // TODO: make sure correct fields and used
	//wod->current_5V = wodTlmStruct->fields->cc1;
	return;
}

void TelemetrySaveEPS(xTimerHandle pxTimer){
	/*
	 imepsv2_piu__gethousekeepingeng__from_t epsTlmStruct;
	 int err = imepsv2_piu__gethousekeepingeng(0,&epsTlmStruct);
	*/
	gom_eps_hkparam_t data_out; // TODO:make sure this is the correct data structure.
	int err = GomEpsGetHkData_param(0,&data_out );
	if(err < 0){
		//TODO: log error
		printf("failed in retrieving EPS house keeping data. (TLM_collector.c / collect_EPS)");
		return;
	}
	WriteData(tlm_eps,data_out.raw);
	/*
	if(err){
		//TODO: log error
		printf("failed in writing EPS house keeping data into file. (TLM_collector.c / collect_EPS)");
		return;
	}*/
	return;
}


void TelemetrySaveTRXVU(xTimerHandle pxTimer){
	return;
	ISIStrxvuTxTelemetry TxTlmStruct;
	ISIStrxvuRxTelemetry RxTlmStruct;
	int err;

	err = IsisTrxvu_tcGetTelemetryAll(ISIS_TRXVU_I2C_BUS_INDEX, &TxTlmStruct);
	if(err){
		//TODO: log error
		printf("failed in retrieving TX house keeping data. (TLM_collector.c / collect_TRX)");
		return;
	}
	WriteData(tlm_tx,TxTlmStruct.raw);
	if(err){
		//TODO: log error
		printf("failed in writing TX house keeping data into file. (TLM_collector.c / collect_EPS)");
		return;
	}



	err = IsisTrxvu_rcGetTelemetryAll(ISIS_TRXVU_I2C_BUS_INDEX,&RxTlmStruct);
	if(err){
		//TODO: log error
		printf("failed in retrieving RX house keeping data. (TLM_collector.c / collect_TRX)");
		return;
	}

	WriteData(tlm_rx,RxTlmStruct.raw);
	if(err){
		//TODO: log error
		printf("failed in writing RX house keeping data into file. (TLM_collector.c / collect_TRX)");
		return;
	}
}

void TelemetrySaveANT(xTimerHandle pxTimer){
	return;
	isis_ants2__get_all_telemetry__from_t antsTlmStruct;
	int err = isis_ants2__get_all_telemetry(0,&antsTlmStruct);
	if(err){
		//TODO: log error
		printf("failed in retrieving EPS house keeping data. (TLM_collector.c / collect_ANTS)");
		return;
	}
	WriteData(tlm_antenna,antsTlmStruct.raw);
	if(err){
		//TODO: log error
		printf("failed in writing EPS house keeping data into file. (TLM_collector.c / collect_ANTS)");
		return;
	}
	return;

}

typedef union _SolarPack{
	unsigned char raw[44];
	struct {
		int32_t temp;
		int8_t status;
		int state;
	} pharsed;
} SolarPack;
void TelemetrySaveSOLAR(xTimerHandle pxTimer){
	return;
	SolarPack sp;
	sp.pharsed.state = IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_0,&sp.pharsed.state,&sp.pharsed.status); //
	WriteData(tlm_solar,sp.raw);
	return;

}



void TelemetrySaveWOD(xTimerHandle pxTimer){
	return;
	imepsv2_piu__gethousekeepingengincdb__from_t epsTlmStruct;
	int err = imepsv2_piu__gethousekeepingengincdb(EPS_I2C_BUS_INDEX,&epsTlmStruct);
	if(err){
		//TODO: log error
		printf("failed in retrieving WOD house keeping data. (TLM_collector.c / collect_WOD)");
		return;
	}
	WriteData(tlm_wod,epsTlmStruct.raw);
	if(err){
		//TODO: log error
		printf("failed in writing WOD house keeping data into file. (TLM_collector.c / collect_WOD)");
		return;
	}
	return;
}

void updateSaveTime(tlm_type_t tlm){
	int sec;
	FRAM_read( (unsigned char *) &sec,  TLM_SAVE_PERIOD_START_ADDR + 4*tlm, 4);
	xTimerChangePeriod(timerArray[tlm],SECONDS_TO_TICKS(sec),0);

}



void TelemetryCollectorLogic(){
	int sec;
	FRAM_read( (unsigned char *)&sec,  EPS_SAVE_TLM_PERIOD_ADDR, 4);
	timerArray[0] = xTimerCreate((const signed char *)"EPS_TIMER",
					SECONDS_TO_TICKS(sec),
					pdTRUE,
					(void*) 0,
					TelemetrySaveEPS);

	FRAM_read( (unsigned char *)&sec,  TRXVU_SAVE_TLM_PERIOD_ADDR, 4);
	timerArray[1] = xTimerCreate((const signed char *)"TRXVU_TIMER",
					SECONDS_TO_TICKS(sec),
					pdTRUE,
					(void*) 1,
					TelemetrySaveTRXVU);

	FRAM_read( (unsigned char *)&sec,  ANT_SAVE_TLM_PERIOD_ADDR, 4);
	timerArray[2] = xTimerCreate((const signed char *)"ANT_TIMER",
					SECONDS_TO_TICKS(sec),
					pdTRUE,
					(void*) 2,
					TelemetrySaveANT);

	FRAM_read( (unsigned char *)&sec,  SOLAR_SAVE_TLM_PERIOD_ADDR, 4);
	timerArray[3] = xTimerCreate((const signed char *)"SOLAR_TIMER",
						SECONDS_TO_TICKS(sec),
						pdTRUE,
						(void*) 3,
						TelemetrySaveSOLAR);

	FRAM_read( (unsigned char *)&sec,  WOD_SAVE_TLM_PERIOD_ADDR, 4);
	timerArray[4] = xTimerCreate((const signed char *)"WOD_TIMER",
							SECONDS_TO_TICKS(sec),
							pdTRUE,
							(void*) 4,
							GetCurrentWODTelemetry);

	for(int i = 0 ; i < 5 ; i++){
		if(timerArray[i] == NULL){
			//TODO: handle timer init fail.
			printf("failed to allocate timers: %d\n",i);
		}
	}
	for(int i = 0 ; i < 5 ; i++){
			if(xTimerStart(timerArray[i],0) != pdPASS){
			//TODO: handle failed timers.
				printf("failed to start timers: %d\n",i);
			}
		}
}

