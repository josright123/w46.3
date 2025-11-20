//#include "../../dm9051_u2510_if/platform_info.h" //"all, as main.c including"

//#undef GET_FIELD
//#undef SET_FIELD
//#define GET_FIELD(field)								dm_mget_##field()	// call-use
//#define SET_FIELD(field, v)							dm_mset_##field(v)	// call-use
//#define IP_TYPES_GET_CSTATE_FUNC(res, rtype, field)		res dm_mget_##field(void)
//#define IP_TYPES_SET_CSTATE_FUNC(res, rtype, field, v)	res dm_mset_##field(const rtype v)

//extern
//typedef uint8_t ip_t[ADDR_LENGTH];
//IP_TYPES_GET_CSTATE_FUNC(uint8_t *, ip_t, final_ip);
//IP_TYPES_SET_CSTATE_FUNC(uint8_t *, ip_t, final_ip, v);
//IP_TYPES_GET_CSTATE_FUNC(uint8_t *, ip_t, final_gw);
//IP_TYPES_SET_CSTATE_FUNC(uint8_t *, ip_t, final_gw, v);
//IP_TYPES_GET_CSTATE_FUNC(uint8_t *, ip_t, final_mask);
//IP_TYPES_SET_CSTATE_FUNC(uint8_t *, ip_t, final_mask, v);

/* APIs.identify
 */
#define ADDR_LENGTH                     (4)
/* static ip, or dedicated for if dhcpc be tries timeout failure */
//const uint8_t local_d1_ip[ADDR_LENGTH] = {192, 168, 1, 117};
//const uint8_t local_d1_gw[ADDR_LENGTH] = {192, 168, 1, 1};
//const uint8_t local_d1_mask[ADDR_LENGTH] = {255, 255, 255, 0};
//const uint8_t local_d6_ip[ADDR_LENGTH] = {192, 168, 6, 117};
//const uint8_t local_d6_gw[ADDR_LENGTH] = {192, 168, 6, 1};
//const uint8_t local_d6_mask[ADDR_LENGTH] = {255, 255, 255, 0};
//#define local_d_ip		local_d6_ip
//#define local_d_gw		local_d6_gw
//#define local_d_mask		local_d6_mask
//const uint8_t local_ip[ADDR_LENGTH] = {192, 168, 6, 117};
//const uint8_t local_gw[ADDR_LENGTH] = {192, 168, 6, 1};
//const uint8_t local_mask[ADDR_LENGTH] = {255, 255, 255, 0};
static const uint8_t local_ip_static_d1[ADDR_LENGTH] = {192, 168, 1, 127};
static const uint8_t local_gw_static_d1[ADDR_LENGTH] = {192, 168, 1, 1};
static const uint8_t local_mask_static_d1[ADDR_LENGTH] = {255, 255, 255, 0};
static const uint8_t local_ip_static_d6[ADDR_LENGTH] = {192, 168, 6, 127};
static const uint8_t local_gw_static_d6[ADDR_LENGTH] = {192, 168, 6, 1};
static const uint8_t local_mask_static_d6[ADDR_LENGTH] = {255, 255, 255, 0};
//struct ip_node_t
//{
//	//uint8_t mac_addresse[MAC_ADDR_LENGTH];
//	uint8_t local_ipaddr[ADDR_LENGTH];
//	uint8_t local_gwaddr[ADDR_LENGTH];
//	uint8_t local_maskaddr[ADDR_LENGTH];
//};
//static const struct ip_node_t local_ip_static_d1[] = {
//	{
//		//{0, 0x60, 0x6e, 0x00, 0x00, 0x17},
//		{192, 168, 1, 117},
//		{192, 168, 1, 1},
//		{255, 255, 255, 0},
//	},
//};
//static const struct ip_node_t local_ip_static_d6[] = {
//	{
//		//{0, 0x60, 0x6e, 0x00, 0x00, 0x17},
//		{192, 168, 6, 117},
//		{192, 168, 6, 1},
//		{255, 255, 255, 0},
//	},
//};

//#define local_ip			local_ip_static_d1 //&local_ip_static_d6[0].local_ipaddr[0]
//#define local_gw			local_gw_static_d6 //&local_ip_static_d6[0].local_gwaddr[0]
//#define local_mask			local_mask_static_d6 //&local_ip_static_d6[0].local_maskaddr[0]

#define local_ip			local_ip_static_d1 //&local_ip_static_d6[0].local_ipaddr[0]
#define local_gw			local_gw_static_d1 //&local_ip_static_d6[0].local_gwaddr[0]
#define local_mask			local_mask_static_d1 //&local_ip_static_d6[0].local_maskaddr[0]

/* IP candidate Configuration */
//static const struct ip_node_t ip_candidate[] = {
//	{
//		//{0, 0x60, 0x6e, 0x00, 0x00, 0x17},
//		{192, 168, 1, 17},
//		{192, 168, 1, 1},
//		{255, 255, 255, 0},
//	},
//	// ... other nodes can be uncommented and added here
//};
//struct ip_data_t
//{
//	uint8_t ipaddr[ADDR_LENGTH];
//};
//static const struct ip_data_t ip_null[] = {
//	{
//		{0, 0, 0, 0},
//	}
//};

//#define candidate_eth_ip() &ip_candidate[0].local_ipaddr[0]     //[pin_code]
//#define candidate_eth_gw() &ip_candidate[0].local_gwaddr[0]     //[pin_code]
//#define candidate_eth_mask() &ip_candidate[0].local_maskaddr[0] //[pin_code]
//#define null_eth_ip() &ip_null[0].ipaddr[0] //[null ip]

//#define identify_tcpip_ip(ip4adr) SET_FIELD1(final_ip, ip4adr, identified_dhcpc_en() ? &ip_null[0].ipaddr[0] : candidate_eth_ip())
//#define identify_tcpip_gw(ip4adr) SET_FIELD1(final_gw, ip4adr, identified_dhcpc_en() ? &ip_null[0].ipaddr[0] : candidate_eth_gw())
//#define identify_tcpip_mask(ip4adr) SET_FIELD(final_mask, ip4adr ? ip4adr : candidate_eth_mask())
//.#define identify_tcpip_ip(ip4adr)	SET_FIELD(final_ip, ip4adr ? ip4adr : identified_dhcpc_en() ? &ip_null[0].ipaddr[0] : candidate_eth_ip())
//.#define identify_tcpip_gw(ip4adr)	SET_FIELD(final_gw, ip4adr ? ip4adr : identified_dhcpc_en() ? &ip_null[0].ipaddr[0] : candidate_eth_gw())
//.#define identify_tcpip_mask(ip4adr)	SET_FIELD(final_mask, ip4adr ? ip4adr : candidate_eth_mask())
