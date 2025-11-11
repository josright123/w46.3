/* dbg_opts.h
 * 
 * Top config
 * Middle settings
 * Lower subroutines
 */
#ifndef __IDENTIFY_OPTS_H__
#define	__IDENTIFY_OPTS_H__

#define ap_print									1 //section
#define rt_print									1 //section (of "dm9051_plat_debug.h")
#define drv_print									1 //section
#define DM_ETH_DEBUG_MODE					1 //0 //1
#define IDENTIFY_PRINTF_IRQ_STATE			0

#define qprint 										(ap_print | rt_print | drv_print)

#endif //__IDENTIFY_OPTS_H__
