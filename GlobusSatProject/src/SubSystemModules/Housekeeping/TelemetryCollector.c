#include "TelemetryCollector.h"

WOD_Telemetry_t dummy_wod = {0};

void GetCurrentWODTelemetry(WOD_Telemetry_t *wod) {
	// dummy for testing
	wod = &dummy_wod;
}
