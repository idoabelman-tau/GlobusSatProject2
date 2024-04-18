
#ifndef GLOBUSSATPROJECT2_EPS_STUB_H
#define GLOBUSSATPROJECT2_EPS_STUB_H
#include "GlobalStandards.h"
#include <satellite-subsystems/imepsv2_piu.h>

void SetUseStub(Boolean use);
void SetVoltage(voltage_t vbat);
int imepsv2_piu__gethousekeepingeng_stub(uint8_t index, imepsv2_piu__gethousekeepingeng__from_t *response);
#endif //GLOBUSSATPROJECT2_EPS_STUB_H
