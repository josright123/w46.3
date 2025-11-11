/*
 * RttPrintf.h
 *
 *  Created on: 19 feb. 2023
 *      Author: perba
 *
 *      This is to help printing on Segger J-Link RTT Viewer.
 *
 *  Configure Simplicity Studio in the *.slps file of your project with:
 *  Services/IO Stream/ Driver / IO Stream:RTT
 *  Third Party/Segger/RTT/SEGGER RTT
 *  Third Party/Segger/RTT/SEGGER RTT printf
 *
 *  In the app_init:
 *  void app_init(void)
 *  {
 *    SEGGER_RTT_Init();
 *  }
 *
 *  Call RttPrintf() instead of printf().
 *  Can be done via the macro below.
 *
 *  For the WGM160P:
 *  Control block is at 0x20001400. This is used by J-link RTT Viewer.
 *
 *  NOTE: RTT should not be present in the release version.
 */
#ifndef RTTPRINTF_H_
#define RTTPRINTF_H_

#include <stdio.h>
#include <stdarg.h>
//.#include "SEGGER_RTT.h"

#ifdef __cplusplus
extern "C" {
#endif
void RttPrintf(const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#define USE_RTT

#ifdef USE_RTT
#define printf RttPrintf
#else
#define SEGGER_RTT_Init()

#endif





#endif /* RTTPRINTF_H_ */
