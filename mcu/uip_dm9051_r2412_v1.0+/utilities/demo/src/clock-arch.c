/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack
 *
 * $Id: clock-arch.c,v 1.2 2006/06/12 08:00:31 adam Exp $
 */

/**
 * \file
 *         Implementation of architecture-specific clock functionality
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "clock-arch.h"
#include "FreeRTOS.h"
//#include "platform_info/nosys/nosys_control/conf_ap.h" //#include "control/conf.h"
#include "../../dm9051_u2510_if/if.h" //"all, as main.c including"
#define AT_hal_tick_count dm9051_boards_heartbeat_now //dm_sys_now
uint32_t AT_hal_tick_count(void);

//extern __IO int32_t g_RunTime;
extern portTickType xTaskGetTickCount(void);

/*---------------------------------------------------------------------------*/
clock_time_t clock_time(void)
{
#if 0
    //return g_RunTime;
    return xTaskGetTickCount();
#else
		//[20241108] Joseph Debug to it, relate to freertos's xTaskGetTickCount().
		return dm9051_boards_heartbeat_now(); //dm_sys_now();
#endif
}
/*---------------------------------------------------------------------------*/
