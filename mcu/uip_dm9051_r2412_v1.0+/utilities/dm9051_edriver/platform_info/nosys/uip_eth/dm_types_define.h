//.#ifndef __DM_TYPES2_H
//.#define __DM_TYPES2_H

#if DM_TYPE == 0

/* HCC: Hard Core Candidate (hcc)
 */
extern const struct ip_data_t ip_null[1];
extern const struct ip_node_t ip_candidate[1];

/* [ identify.h/identify.c ] */
#define GET_FIELD(field) dm_mget_##field()		   // call-use
#define SET_FIELD(field, val) dm_mset_##field(val) // call-use
#define SET_FIELD1(field, inval,candi) dm_mset_##field(inval,candi) // call-use

#undef DM_RMACRO
#define DM_RMACRO(rtype, mtype, field) \
	rtype dm_mget_##field(void);       \
	rtype dm_mset_##field(const mtype adr);

#undef DM_RMACRO1
#define DM_RMACRO1(rtype, mtype, field) \
	rtype dm_mget_##field(void);       \
	rtype dm_mset_##field(const mtype adr, const mtype candi);

//#define GET_CSTATE(field) cb_get_##field()	   // call-use
//#define SET_CSTATE(field, v) cb_set_##field(v) // call-use

//#define CB_TYPES_GET_CSTATE_FX(field) cb_get_##field(void)
//#define CB_TYPES_SET_CSTATE_FX(mtype, field, v) cb_set_##field(const mtype v)

//#undef CB_MACRO
//#define CB_MACRO(mtype, field)           \
//	mtype CB_TYPES_GET_CSTATE_FX(field); \
//	mtype CB_TYPES_SET_CSTATE_FX(mtype, field, v);

/* APIs.identify
 */
#define identify_dhcpc_en(val) SET_FIELD(dhcpc_en, val)
#define identified_dhcpc_en() GET_FIELD(dhcpc_en)

//#define candidate_eth_mac() &node_candidate[0].mac_addresse[0]    //[pin_code]
#define candidate_eth_ip() &ip_candidate[0].local_ipaddr[0]     //[pin_code]
#define candidate_eth_gw() &ip_candidate[0].local_gwaddr[0]     //[pin_code]
#define candidate_eth_mask() &ip_candidate[0].local_maskaddr[0] //[pin_code]
// const uint8_t *identify_eth_mac(const uint8_t *macadr);
// uint8_t *identify_tcpip_ip(uint8_t *ip4adr);
// uint8_t *identify_tcpip_gw(uint8_t *ip4adr);
// uint8_t *identify_tcpip_mask(uint8_t *ip4adr);
// void trace_identify_eth_mac(void);
//#define identify_eth_mac(macadr) SET_FIELD(final_mac, macadr ? macadr : candidate_eth_mac())
//#define identify_tcpip_ip(ip4adr) SET_FIELD1(final_ip, ip4adr ? ip4adr : candidate_eth_ip())
//#define identify_tcpip_gw(ip4adr) SET_FIELD1(final_gw, ip4adr ? ip4adr : candidate_eth_gw())
#define identify_tcpip_ip(ip4adr) SET_FIELD1(final_ip, ip4adr, identified_dhcpc_en() ? &ip_null[0].ipaddr[0] : candidate_eth_ip())
#define identify_tcpip_gw(ip4adr) SET_FIELD1(final_gw, ip4adr, identified_dhcpc_en() ? &ip_null[0].ipaddr[0] : candidate_eth_gw())
#define identify_tcpip_mask(ip4adr) SET_FIELD(final_mask, ip4adr ? ip4adr : candidate_eth_mask())

/* APIs.identified
 */
// const uint8_t *identified_eth_mac(void);
// uint8_t *identified_tcpip_ip(void);
// uint8_t *identified_tcpip_gw(void);
// uint8_t *identified_tcpip_mask(void);
//#define identified_eth_mac() GET_FIELD(final_mac)
#define identified_tcpip_ip() GET_FIELD(final_ip)
#define identified_tcpip_gw() GET_FIELD(final_gw)
#define identified_tcpip_mask() GET_FIELD(final_mask)

#if 1
	DM_RMACRO(int, int, dhcpc_en);
//	DM_RMACRO(uint8_t *, mac_t, final_mac);
	DM_RMACRO1(uint8_t *, ip_t, final_ip);
	DM_RMACRO1(uint8_t *, ip_t, final_gw);
	DM_RMACRO(uint8_t *, ip_t, final_mask);

//	CB_MACRO(uint16_t, irqst);

	/* irqstate.identify
	 */
//.	const char *level_str_impl(dm9051_eth_debug_level_t level);
#endif

#endif

#if DM_TYPE == 1

const struct ip_data_t ip_null[1] = {
	{
		{0, 0, 0, 0},
	}
};

/* IP candidate Configuration */
const struct ip_node_t ip_candidate[1] = {
	{
		//{0, 0x60, 0x6e, 0x00, 0x00, 0x17},
		{192, 168, 6, 17},
		{192, 168, 6, 1},
		{255, 255, 255, 0},
	}, 
	/*
	{
	 //{0, 0x60, 0x6e, 0x00, 0x00, 0x25},
	 {192, 168, 6,  25},
	 {192, 168, 6,   1},
	 {255, 255, 255, 0},
	},
	   */
	// ... other nodes can be uncommented and added here
};

#undef DM_MACRO
#define DM_MACRO(mtype, field) \
	mtype field;

static struct dmtype_data
{
	DM_MACRO(int, dhcpc_en)
	//  mac_t final_mac;
	//	ip_t final_ip;
	//	ip_t final_gw;
	//	ip_t final_mask;
	//DM_MACRO(mac_t, final_mac)
	DM_MACRO(ip_t, final_ip)
	DM_MACRO(ip_t, final_gw)
	DM_MACRO(ip_t, final_mask)
} dm;

#elif DM_TYPE == 2

/* essential extern sub */
static void print_versal_configuration(char *head, char *adr_name, const uint8_t *ip)
{
#if rt_print | drv_print
	const int eth_pnt = 0;
	char buf[100];
  sprintf(buf, "%s %s %d.%d.%d.%d\r\n", head, adr_name, ip[0], ip[1], ip[2], ip[3]);
	//printky(buf);
	if (eth_pnt)
		eth_printf(buf);
	else
		eth_printkey(buf);
#endif
}

#undef DM_RMACRO1
#define DM_RMACRO1(rtype, mtype, field, adr_len, adr_name)                               \
	rtype dm_mget_##field(void)                                    \
	{                                               						\
		return dm.field;                                                      \
	}                                                                         \
	rtype dm_mset_##field(const mtype adr, const mtype candi)                         \
	{                                  \
		static uint8_t printag_##field = 0x1;	\
		\
		memcpy(dm.field, adr ? adr : candi, adr_len);	\
		if (printag_##field & 0x01) {	\
			printag_##field &= ~0x01;	\
			/*dm_eth_show_identified_ip/dm_eth_show_identified_gw(adr ? "config ip" : "candidate ip");= */	\
			print_versal_configuration(adr ? "config" : "candidate", adr_name, dm.field); \
		}	\
		return dm.field;                                                      \
	}

#undef DM_RMACRO
#define DM_RMACRO(rtype, mtype, field, adr_len)                               \
	rtype dm_mget_##field(void)                                    \
	{                                               						\
		return dm.field;                                                      \
	}                                                                         \
	rtype dm_mset_##field(const mtype adr)                         \
	{                                  \
		memcpy(dm.field, adr, adr_len);                                       \
		return dm.field;                                                      \
	}

#undef DM_MACRO
#define DM_MACRO(rtype, mtype, field)                               \
	rtype dm_mget_##field(void)                                    \
	{                                               						\
		return dm.field;                                                      \
	}                                                                         \
	rtype dm_mset_##field(const mtype val)                         \
	{                                  \
		dm.field = val;                                       \
		return dm.field;                                                      \
	}

DM_MACRO(int, int, dhcpc_en)
//DM_RMACRO(uint8_t *, mac_t, final_mac, MAC_ADDR_LENGTH)
DM_RMACRO1(uint8_t *, ip_t, final_ip, ADDR_LENGTH, "ip")
DM_RMACRO1(uint8_t *, ip_t, final_gw, ADDR_LENGTH, "gw")
DM_RMACRO(uint8_t *, ip_t, final_mask, ADDR_LENGTH)

/* Debug Level Implementation */
//const char *level_str_impl(dm9051_eth_debug_level_t level)
//{
//    static const char* const LEVEL_STRINGS[] = {
//        "ERROR", //[DM9051_ETH_DEBUG_LEVEL_ERROR] = 
//        "DEBUG", //[DM9051_ETH_DEBUG_LEVEL_DEBUG] = 
//        "WARN", //[DM9051_ETH_DEBUG_LEVEL_WARN]  = 
//        "INFO", //[DM9051_ETH_DEBUG_LEVEL_INFO]  = 
//    };

//    return (level < sizeof(LEVEL_STRINGS)/sizeof(LEVEL_STRINGS[0]) && 
//            LEVEL_STRINGS[level]) ? LEVEL_STRINGS[level] : "UNKNOWN";
//}
#endif

#undef DM_TYPE
//.#endif //__DM_TYPES2_H
