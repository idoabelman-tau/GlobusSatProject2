#include "EpsStub.h"
#include <string.h>
#include "SubSystemModules/PowerManagement/EPS.h"

voltage_t mock_voltage_data = 0;
Boolean use_stub = TRUE;

void SetUseStub(Boolean use) {
    use_stub = use;
}
void SetVoltage(voltage_t vbat) {
    mock_voltage_data = vbat;
}
int GetBatteryVoltage_stub(voltage_t *vbat) {
    if (use_stub) {
        *vbat = mock_voltage_data;
        return 0;
    } else {
#ifdef GOMEPS
    	return GetBatteryVoltage_gom(vbat);
#elif defined(ISISEPS)
    	return GetBatteryVoltage_isis(vbat);
#endif
    }
}

