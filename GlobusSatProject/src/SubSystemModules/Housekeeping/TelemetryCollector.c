#include "TelemetryCollector.h"

WOD_Telemetry_t dummy_wod = {0};

void GetCurrentWODTelemetry(WOD_Telemetry_t *wod) {
	// dummy for testing
	wod = &dummy_wod;
}

void TelemetrySaveEPS(){
	/*
	 imepsv2_piu__gethousekeepingeng__from_t epsTlmStruct;
	 int err = imepsv2_piu__gethousekeepingeng(0,&epsTlmStruct);
	*/
	gom_eps_hk_vi_t data_out;
	int err = GomEpsGetHkData_vi(0,&data_out );
	if(err < 0){
		//TODO: log error
		printf("failed in retrieving EPS house keeping data. (TLM_collector.c / collect_EPS)");
		return;
	}
	WriteData(tlm_eps,data_out.raw);

	return;
}


void TelemetrySaveTRXVU(){
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



	err = IsisTrxvu_rcGetTelemetryAll(ISIS_TRXVU_I2C_BUS_INDEX,&RxTlmStruct);
	if(err){
		//TODO: log error
		printf("failed in retrieving RX house keeping data. (TLM_collector.c / collect_TRX)");
		return;
	}

	WriteData(tlm_rx,RxTlmStruct.raw);

}

void TelemetrySaveANT(){
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



void TelemetrySaveWOD(){
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


void TelemetryCollectorLogic(){

	TelemetrySaveEPS();


	//TelemetrySaveTRXVU();

	//TelemetrySaveANT();

	//TelemetrySaveWOD();

	//collect_SP();


}

