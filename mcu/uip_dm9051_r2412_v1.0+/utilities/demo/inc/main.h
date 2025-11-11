/**
  **************************************************************************
  * @file     main.h
  * @version  v2.0.0
  * @date     2024-10-20
  * @brief    This file contains all the functions prototypes for the main.c 
  *           file.
  **************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

//void SysTick_Handler_from_main(void); 
  //JJ0 (should be implemented in "..\..\..\middlewares\freertos\source\portable\rvds\ARM_CM4F\port.c")
  //JJ0 (should be in header file "FreeRTOSConfig.h")

#define main_tick_handler SysTick_Handler //JJ0 (in "main.c")

//uint32_t main_tick_count(void); //JJ0 (normal extern)

#endif /* __MAIN_H */
