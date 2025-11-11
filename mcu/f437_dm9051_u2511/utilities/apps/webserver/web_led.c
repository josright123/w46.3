#include "uip.h"
#include "timer.h"
#include "web_led.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "sl_udelay.h"
#include "sl_simple_led.h"
#include "sl_simple_led_instances.h"
char sCurrLED[4] = {'o', 'n', 0};
char sCurrP05[6] = {'o', 'f', 'f', 0};

#ifndef LED1_INSTANCE
#define LED1_INSTANCE               sl_led_led1
#endif

int CurrTmp = 0;
int CurrHum = 0;

uint32_t LED_timer_flag;
extern int Web_LED_FLASH;

void Delay(uint32_t times)
{
    while (times--)
    {
        uint32_t i;

        for (i = 0; i < 0xffff; i++);
    }
}

void Set_LED_mode(char lkkcode)
{
    //int i;

    if (lkkcode == ('0')) // LED off
    {
        Web_LED_FLASH = 0;
        //Delay(25);
        vTaskDelay(500 / portTICK_RATE_MS);
        sl_led_turn_off(&LED1_INSTANCE);
    }
    else if (lkkcode == '1')   // LED on
    {
        Web_LED_FLASH = 0;
        sl_led_turn_on(&LED1_INSTANCE);
    }
    else if (lkkcode == '2')  // LED Flash
    {
        Web_LED_FLASH = 0;
        //for(i = 0 ; i< 30 ; ++i)
        {
            sl_led_turn_on(&LED1_INSTANCE);
            //Delay(25);
            vTaskDelay(500 / portTICK_RATE_MS);
            sl_led_turn_off(&LED1_INSTANCE);
            //Delay(25);
            vTaskDelay(500 / portTICK_RATE_MS);
        }
    }
    else if (lkkcode == '3')
    {
        Web_LED_FLASH = 1; // if web control over, return freertos control
    }
}
