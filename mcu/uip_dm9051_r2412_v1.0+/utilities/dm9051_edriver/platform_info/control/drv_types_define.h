//.#ifndef __DRV_TYPES2_H
//.#define __DRV_TYPES2_H

#if DRV_TYPE == 0

/* [ identify.h/identify.c ] */
#define DR_GET_FIELD(field) dr_mget_##field()		   // call-use
#define DR_SET_FIELD(field, val) dr_mset_##field(val) // call-use

#define GET_CSTATE(field) cb_get_##field()	   // call-use
#define SET_CSTATE(field, v) cb_set_##field(v) // call-use

#define CB_TYPES_GET_CSTATE_FX(field) cb_get_##field(void)
#define CB_TYPES_SET_CSTATE_FX(mtype, field, v) cb_set_##field(const mtype v)

#undef DR_RMACRO
#define DR_RMACRO(rtype, mtype, field) \
	rtype dr_mget_##field(void);       \
	rtype dr_mset_##field(const mtype adr);

#undef CB_MACRO
#define CB_MACRO(mtype, field)           \
	mtype CB_TYPES_GET_CSTATE_FX(field); \
	mtype CB_TYPES_SET_CSTATE_FX(mtype, field, v);

/* HCC: Hard Core Candidate (hcc)
 */
extern const struct eth_node_t node_candidate[1];

DR_RMACRO(uint8_t *, mac_t, final_mac);

CB_MACRO(uint16_t, irqst);

/*
 * identify_mac
 */

#define identified_eth_mac() DR_GET_FIELD(final_mac)
#define identify_eth_mac(macadr) DR_SET_FIELD(final_mac, macadr ? macadr : candidate_eth_mac())
#define trace_identify_eth_mac()                            \
  do                                                        \
  {                                                         \
    const uint8_t *mac = DR_GET_FIELD(final_mac);              \
	  printf("mac address %02x:%02x:%02x:%02x:%02x:%02x\r\n",      \
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]); \
  } while (0)

#define candidate_eth_mac() &node_candidate[0].mac_addresse[0]    //[pin_code]

/*
 * trace_irq_flow
 */

#define ISTAT_IRQ_CFG (1 << 0)
#define ISTAT_IRQ_ENAB (1 << 1)
#define ISTAT_DM_IMR (1 << 2)
#define ISTAT_DM_RCR (1 << 3)

#define ISTAT_LOW_TRIGGER (1 << 4)
#define ISTAT_LOW_ACTIVE (1 << 5)

#define ISTAT_IRQ_NOW (1 << 6)
#define ISTAT_IRQ_NOT_NOW (1 << 7)

#define ISTAT_IRQ_NOW2 (1 << 8)
#define ISTAT_IRQ_NOW2END (1 << 9)

#define deidentify_irq_stat(bitflg) SET_CSTATE(irqst, GET_CSTATE(irqst) & ~bitflg)
#define identify_irq_stat(bitflg) SET_CSTATE(irqst, GET_CSTATE(irqst) | bitflg)
#define identified_irq_stat() GET_CSTATE(irqst)

/* irqstate.identified
 */
#include "platform_info/control/dbg_opts.h"

#if IDENTIFY_PRINTF_IRQ_STATE
// void trace_irq_stat(uint16_t bitflg);
#define trace_irq_stat(bitflg)                                             \
	do                                                                     \
	{                                                                      \
		char istat_term[22];                                               \
		switch (bitflg)                                                    \
		{                                                                  \
		case ISTAT_IRQ_CFG:                                                \
			sprintf(istat_term, "[IRQ_CFG]");                              \
			break;                                                         \
		case ISTAT_IRQ_ENAB:                                               \
			sprintf(istat_term, "[IRQ_ENAB]");                             \
			break;                                                         \
		case ISTAT_DM_IMR:                                                 \
			sprintf(istat_term, "(IMR.pr)");                               \
			break;                                                         \
		case ISTAT_DM_RCR:                                                 \
			sprintf(istat_term, "(RCR.rxen)");                             \
			break;                                                         \
		case ISTAT_LOW_TRIGGER:                                            \
			sprintf(istat_term, "[IRQ_LOW_TRIGGER]");                      \
			break;                                                         \
		case ISTAT_LOW_ACTIVE:                                             \
			sprintf(istat_term, "(INTR.lo)");                              \
			break;                                                         \
		case ISTAT_IRQ_NOW:                                                \
			sprintf(istat_term, "(ISrvRoutine)");                          \
			break;                                                         \
		case ISTAT_IRQ_NOT_NOW:                                            \
			sprintf(istat_term, "(ISrvRoutine NOT match)");                \
			break;                                                         \
		case ISTAT_IRQ_NOW2:                                               \
			sprintf(istat_term, "(INT %lu)", dm_eth_interrupt_count());       \
			break;                                                         \
		case ISTAT_IRQ_NOW2END:                                            \
			sprintf(istat_term, "(INT.End)");                              \
			break;                                                         \
		default:                                                           \
			istat_term[0] = 0;                                             \
			break;                                                         \
		}                                                                  \
		if (dm_eth_interrupt_count() > 5 && bitflg == ISTAT_IRQ_NOW)          \
			;                                                              \
		else if (dm_eth_interrupt_count() > 5 && bitflg == ISTAT_IRQ_NOW2)    \
		{                                                                  \
			if (!(dm_eth_interrupt_count() % 25))                             \
				printf("%s--> %s\r\n", PRINT_INFO_IRQ, istat_term);              \
		}                                                                  \
		else if (dm_eth_interrupt_count() > 5 && bitflg == ISTAT_IRQ_NOW2END) \
			printf("%s--> %s\r\n", PRINT_INFO_IRQ, istat_term);                  \
		else                                                               \
			printf("%s--> irqst= %02x on add-bit %02x %s\r\n",   \
				   PRINT_INFO_IRQ, GET_CSTATE(irqst), bitflg, istat_term);                 \
	} while (0)
#else
#define trace_irq_stat(bitflg)
#endif

const char *level_str_impl(dm9051_eth_debug_level_t level);
#endif


#if DRV_TYPE == 1

#undef DR_MACRO
#define DR_MACRO(mtype, field) \
	mtype field;
	
/* Network candidate Configuration */
const struct eth_node_t node_candidate[1] = {
	{
		{0, 0x60, 0x6e, 0x00, 0x00, 0x17},
	}, 
	/*
	{
	 {0, 0x60, 0x6e, 0x00, 0x01, 0x25,},
	},
	   */
	// ... other nodes can be uncommented and added here
};

static struct cbtype_data
{
	uint16_t irqst;
} cb = {
	0x0000,
};

static struct drtype_data
{
	DR_MACRO(mac_t, final_mac)
} dr;
#endif


#if DRV_TYPE == 2

#define CB_TYPES_GET_CSTATE_FUNC(field) cb_get_##field(void)
#define CB_TYPES_SET_CSTATE_FUNC(mtype, field, v) cb_set_##field(const mtype v)

#undef CB_MACRO
#define CB_MACRO(mtype, field)                      \
	mtype CB_TYPES_GET_CSTATE_FUNC(field)           \
	{                                               \
		return cb.field;                            \
	}                                               \
	mtype CB_TYPES_SET_CSTATE_FUNC(mtype, field, v) \
	{                                               \
		cb.field = v;                               \
		return v;                                   \
	}

CB_MACRO(uint16_t, irqst);

#undef DR_RMACRO
#define DR_RMACRO(rtype, mtype, field, adr_len)                               \
	rtype dr_mget_##field(void)                                    \
	{                                               						\
		return dr.field;                                                      \
	}                                                                         \
	rtype dr_mset_##field(const mtype adr)                         \
	{                                  \
		memcpy(dr.field, adr, adr_len);                                       \
		return dr.field;                                                      \
	}

DR_RMACRO(uint8_t *, mac_t, final_mac, MAC_ADDR_LENGTH)

/* Debug Level Implementation */
const char *level_str_impl(dm9051_eth_debug_level_t level)
{
    return (level < sizeof(LEVEL_STRINGS)/sizeof(LEVEL_STRINGS[0]) && 
            LEVEL_STRINGS[level]) ? LEVEL_STRINGS[level] : "UNKNOWN";
}
#endif

#undef DRV_TYPE
//.#endif //__DRV_TYPES2_H
