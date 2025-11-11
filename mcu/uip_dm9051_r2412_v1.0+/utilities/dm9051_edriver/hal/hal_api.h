#ifndef __HAL_API_MCU_H
#define __HAL_API_MCU_H

//	option: _DLW_M051xx
//	option: _DLW_AT32F437xx
//	option: _DLW_AT32F415xx
	#define _DLW_AT32F437xx

#ifdef __HAL_API_MCU_H
//	apply to config MCU _DLW_M051xx
//	apply to config MCU _DLW_AT32F437xx
//	apply to config MCU _DLW_AT32F415xx, ...,etc.
#if defined(_DLW_M051xx)
#include "M051Series.h"
#elif defined(_DLW_AT32F437xx)
#include "at32f435_437_board.h" //mcu's board
#include "at32f435_437_clock.h" //Also mcu's clock
#elif defined(_DLW_AT32F415xx)
#include "at32f415_board.h" //mcu's board
#include "at32f415_clock.h" //Also mcu's clock
#elif defined(_DLW_AT32F413xx)
#include "at32f413_board.h" //mcu's board
#include "at32f413_clock.h" //Also mcu's clock
#elif defined(_DLW_AT32F403Axx) || defined(_DLW_AT32F403xx) || defined(_DLW_AT32F407xx)
#include "at32f403a_407_board.h" //mcu's board
#include "at32f403a_407_clock.h" //Also mcu's clock
#else
/*
	#error "opts board to be define"
	While NOT include the mcu-definition in the program code, in advance, previously.
	Add your board's board & clock header files here! Refer to above cases.

	Usually, Find the expected included files below in main.c
*/
#error "Opps, Opts board to be define  \r\n \
		While NOT include the mcu-definition in the program code, in advance, previously. \r\n \
		Usually, Project find the expected included files below in Options for Target > C/C++ > Preprocessor Definitions > Add > _AT32F4xxxx \r\n \
		ex: _AT32F407xx,USE_STDPERIPH_DRIVER,AT_START_F407_V1 \r\n \
		ex: _AT32F437xx,USE_STDPERIPH_DRIVER,AT_START_F437_V1 \r\n \ "
#endif
#endif //__HAL_API_MCU_H

typedef enum {VIA_BUTTON = 0, VIA_NET} trigger_type; //#define LED_VIA_BUTTON 0 //#define LED_VIA_LINK 1
typedef enum {LED_OFF = 0, LED_FLASH = !LED_OFF} led_ops_state;

// Interrupt Configuration Structure
struct interrupt_pack_t
{
	/*struct gpio_config_t gpio; */
	struct interrupt_config_t {
		crm_periph_clock_type scfgclock;
		crm_periph_clock_type clock;
		scfg_port_source_type port_source;
		scfg_pins_source_type pin_source;
		uint32_t line;
		nvic_priority_group_type priority_group;
		IRQn_Type irqn;
	} cf;
	exint_polarity_config_type polarity;
};

// general programable gpio input/output Structure
struct gpio_config_t
{
	gpio_type *port;
	uint16_t pin;
	crm_periph_clock_type clock;
	struct {
		gpio_pull_type pull;
		gpio_mode_type mode;
		gpio_pins_source_type source;
		gpio_mux_sel_type mux;
	} mods;
};

struct board_init_type
{
	const struct spi_config_t *spip;
	const struct gpio_config_t *sck;
	const struct gpio_config_t *mi;
	const struct gpio_config_t *mo;
	const struct gpio_config_t *csp;

	int	interrrpt_mode;
	const struct gpio_config_t *gpiop;
	const struct interrupt_pack_t *intp; //uint32_t line;
};

#define AT_interrupt_config_init dm9051if_intr_config
void AT_interrupt_config_init(const struct interrupt_pack_t *pack);

#define AT_hal_stdpin_config dm9051if_stdpin_config //..........
void AT_hal_stdpin_config(const struct gpio_config_t *gpio); //..........
//#define AT_hal_muxpin_config dm9051if_muxpin_config
//void AT_hal_muxpin_config(const struct gpio_config_t *gpiomux);

#define AT_hal_stdpin_lo dm9051if_stdpin_lo
#define AT_hal_stdpin_hi dm9051if_stdpin_hi
void AT_hal_stdpin_lo(const struct gpio_config_t *gpio);
void AT_hal_stdpin_hi(const struct gpio_config_t *gpio);
//#define _AT_hal_stdpin_lo gpio_hal_stdpin_lo
//#define _AT_hal_stdpin_hi gpio_hal_stdpin_hi

#define AT_hal_stdpin_get gpio_stdpin_get
flag_status AT_hal_stdpin_get(const struct gpio_config_t *gpio);

#define AT_hal_disable_mcu_irq cint_disable_mcu_irq
void AT_hal_disable_mcu_irq(void);
#define AT_hal_enable_mcu_irq cint_enable_mcu_irq
void AT_hal_enable_mcu_irq(void);

void config_led3(void);
void led3_toggle(void);
void led3_on(void);
void led3_off(void);
void config_button(void);
button_type button_is_pressed(void);
void config_diag(void);
void diag_lo(void);
void diag_hi(void);
void config_inpt(void);
flag_status inpt_get(void);

void operate_led3(trigger_type trigger, led_ops_state ops);

/* hal_main api
 */
void dm9051_boards_get_info(struct board_init_type *board_init_struct);

#define AT_hal_init dm9051_boards_initialize
void AT_hal_init(void); //(struct board_init_type *board_init_struct);
void AT_hal_intr_init(struct board_init_type *info);

#define AT_hal_tick dm9051_boards_heartbeat_tick
void AT_hal_tick(void);

#define AT_hal_tick_count dm9051_boards_heartbeat_now //dm_sys_now
uint32_t AT_hal_tick_count(void);

/* hal_intr api
 */
#define intr_gpo() &intr_gpio
#define intr_set() &intr_cset[0]
//#define irq_line() intr_cset[0].cf.line
//#define nvic_irqn() intr_cset[0].cf.irqn
//#define nvic_prio() intr_cset[0].cf.priority_group

extern struct board_init_type dm_init_info; //temp here!
extern const struct interrupt_pack_t intr_cset[1];

/* hal_gpio api
 */
extern const struct gpio_config_t cs_gpio;
extern const struct gpio_config_t intr_gpio;

#endif //__HAL_API_MCU_H
