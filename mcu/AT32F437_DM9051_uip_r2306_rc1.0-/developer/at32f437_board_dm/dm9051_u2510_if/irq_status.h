//#include "../../dm9051_u2510_if/platform_info.h" //"all, as main.c including"

#if !DM_ETH_DEBUG_MODE
static void inc_interrupt_count(void) {
}
unsigned long dm_eth_interrupt_count(void) {
    return 0;
}
#endif //!DM_ETH_DEBUG_MODE
#if DM_ETH_DEBUG_MODE
/* Interrupt Tracking */
static volatile unsigned long dispc_int_active = 0;

static void inc_interrupt_count(void)
{
    dispc_int_active++;
}
unsigned long dm_eth_interrupt_count(void)
{
    return dispc_int_active;
}
#endif //DM_ETH_DEBUG_MODE
