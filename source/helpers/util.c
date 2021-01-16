#include "helpers/util.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

#define MHz *1E6

static HidsysUniquePadId g_uniquePadIds[2]; // HidsysUniquePadId was u64 change for new libnx
static s32 g_uniquePadCnt;
static HidsysNotificationLedPattern g_patternOn, g_patternOff;

bool isServiceRunning(const char *serviceName) {
  u8 tmp=0;
  SmServiceName service_name = smEncodeName(serviceName);
  Result rc = serviceDispatchInOut(smGetServiceSession(), 65100, service_name, tmp);
  if (R_SUCCEEDED(rc) && tmp & 1)
    return true;
  else
    return false;
}

void getCurrTimeString(char *buffer) {
  time_t unixTime = time(NULL);
  struct tm* time = localtime((const time_t*)&unixTime);
  sprintf(buffer, "%02d:%02d", time->tm_hour, time->tm_min);
}

void getCurrBatteryPercentage(char *buffer) {
  u32 percents = 0;
  psmGetBatteryChargePercentage(&percents);
  sprintf(buffer, "%d%%", percents);
}
s32 total_entries;
HidsysUniquePadId unique_pad_ids[2] = {0};
void ledInit() {
  // hidsysGetUniquePadsFromNpad(hidGetHandheldMode() ? CONTROLLER_HANDHELD : CONTROLLER_PLAYER_1, g_uniquePadIds, 2, &g_uniquePadCnt);
  // hidsysGetUniquePadsFromNpad(hidGetHandheldMode() ? CONTROLLER_HANDHELD : CONTROLLER_PLAYER_1, g_uniquePadIds, 2, &g_uniquePadCnt);
    // PadState pad;
    // padInitializeDefault(&pad);
  hidsysGetUniquePadsFromNpad(HidNpadIdType_Handheld, g_uniquePadIds, 2, &g_uniquePadCnt);
  hidsysGetUniquePadsFromNpad(HidNpadIdType_No1, unique_pad_ids, 2, &total_entries);
  memset(&g_patternOn, 0x00, sizeof(HidsysNotificationLedPattern));
  memset(&g_patternOff, 0x00, sizeof(HidsysNotificationLedPattern));

  g_patternOn.baseMiniCycleDuration = 0x0F;
  g_patternOn.startIntensity = 0x0F;
  g_patternOn.miniCycles[0].ledIntensity = 0x0F;
  g_patternOn.miniCycles[0].transitionSteps = 0x0F;
  g_patternOn.miniCycles[0].finalStepDuration = 0x0F;
}

void setLedState(bool state) {
  for(u8 i = 0; i < g_uniquePadCnt; i++)
    hidsysSetNotificationLedPattern(state ? &g_patternOn : &g_patternOff, g_uniquePadIds[i]);
  for (u8 i = 0; i < total_entries; i++)
    hidsysSetNotificationLedPattern(state ? &g_patternOn : &g_patternOff, unique_pad_ids[i]);
}

void overclockSystem(bool enable) {
  if (hosversionBefore(8, 0, 0)) {
    pcvSetClockRate(PcvModule_CpuBus, enable ? 1785 MHz : 1020 MHz);  // Set CPU clock
    pcvSetClockRate(PcvModule_EMC, enable ? 1600 MHz : 1331 MHz);     // Set memory clock
  } else {
    ClkrstSession clkrstSession;
    clkrstOpenSession(&clkrstSession, PcvModuleId_CpuBus, 3);
    clkrstSetClockRate(&clkrstSession, enable ? 1785 MHz : 1020 MHz); // Set CPU clock
    clkrstCloseSession(&clkrstSession);

    clkrstOpenSession(&clkrstSession, PcvModuleId_EMC, 3);
    clkrstSetClockRate(&clkrstSession, enable ? 1600 MHz : 1331 MHz); // Set memory clock
    clkrstCloseSession(&clkrstSession);
  }
}
