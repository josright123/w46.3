/*
 * RttPrintf.c
 *
 *  Created on: 21 feb. 2023
 *      Author: perba
 * 
 *  This is to help printing on Segger J-Link RTT Viewer.
 *  Please see the header file for more info.
 */


#include <stdio.h>
#include <stdarg.h>

#if 1
void RttPrintf(const char* fmt, ...) // C-style "const char* fmt, ..." is also valid
{
  va_list args;

  va_start(args, fmt);
  printf(fmt, args);
  va_end(args);
  
//  char buf[160];
//  va_list args;

//  va_start(args, fmt);
//  vsprintf(buf, fmt, args);
//  SEGGER_RTT_printf(0, "%s", buf);

//  va_end(args);
}
#endif
