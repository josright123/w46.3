#ifndef UIP_TASK_H
#define UIP_TASK_H

#include "FreeRTOS.h"

/* The task that handles all uIP data. */
void vuIP_Task(void *pvParameters);
//void vStart_uIP_Tasks( UBaseType_t uxPriority );

#endif

