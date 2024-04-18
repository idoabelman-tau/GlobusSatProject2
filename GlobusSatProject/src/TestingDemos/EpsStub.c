#include "EpsStub.h"
#include <string.h>

imepsv2_piu__gethousekeepingeng__from_t mock_voltage_data = {0};
Boolean use_stub = TRUE;

void SetUseStub(Boolean use) {
    use_stub = use;
}
void SetVoltage(voltage_t vbat) {
    mock_voltage_data.fields.batt_input.fields.volt = vbat;
}
int imepsv2_piu__gethousekeepingeng_stub(uint8_t index, imepsv2_piu__gethousekeepingeng__from_t *response) {
    if (use_stub) {
        memcpy(response, &mock_voltage_data, sizeof(mock_voltage_data));
        return 0;
    } else {
        return imepsv2_piu__gethousekeepingeng(index,response);
    }
}

