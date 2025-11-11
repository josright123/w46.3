/* define to prevent recursive inclusion -------------------------------------*/
#ifndef __DM9051_H
#define __DM9051_H
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>

#define PHY_STATUS_REG            (0x01)           /*!< basic mode status register */
#define DM9051_MRCMD              (0x72)
#define DM9051_MWCMD              (0x78)
#define OPC_REG_W                 0x80             // Register Write
#define OPC_REG_R                 0x00             // Register Read

#define INPUT_MODE_POLL 0
#define INPUT_MODE_INTERRUPT 1
#define INPUT_MODE_INTERRUPT_CLKOUT 2

int dm9051_conf(void);
const uint8_t *dm9051_init(const uint8_t *adr);
uint16_t dm9051_rx(uint8_t *buf);
void dm9051_tx(uint8_t *buf, uint16_t len);
//void dm9051_rx_mode_add_hash(const uint32_t ip_addr);
//void dm9051_rx_mode_del_hash(const uint32_t ip_addr);
void dm9051_igmp_ctrl(const uint32_t group, int op_action);
void dm9051_rx_rcr_all(void);
void dm9051_read_regs_info(uint8_t *stat);
uint16_t dm9051_link_update(void);
void dm9051_read_rx_pointers(uint16_t *rwpa_wt, uint16_t *mdra_rd);
uint16_t cspi_phy_read(uint16_t uReg);

char *hal_get_dm9051_date(void);
char *hal_get_dm9051_release_num(void);
int hal_active_interrupt_mode(void);
char *hal_active_interrupt_desc(void);
void hal_active_interrupt_info(void);
void dm9051_interrupt_set(uint32_t exint_line);
int dm9051_interrupt_get(void);
void dm9051_interrupt_reset(void);

void ctick_delay_us(uint32_t nus);
void ctick_delay_ms(uint16_t nms);
void ncr_compl_timeout(uint32_t timeout_ms);
void tx_pointer_timeout(uint32_t timeout_us);
void tx_compl_timeout(uint8_t tcr_wr, uint32_t timeout_us);

//#define HAL_IRQLine			hal_irqline
//uint32_t hal_irqline(void);

/* internal */
#define DRIVER_VERSION "dm9051_edriver %s"
static char version_string[60];

static void ver_string_get(char *ver_buf, char *path_info)
{
	int found = 0;
	int i, j = 0, n = strlen(path_info);

	for (i = 0; i < n; i++)
	{
		if (found && path_info[i] == '\\')
			break;
		if (path_info[i] == 'v' && path_info[i - 1] == '_')
		{
			found = 1;
			j = 0;
			ver_buf[j++] = path_info[i];
			continue;
		}
		ver_buf[j++] = path_info[i];
	}
	ver_buf[j++] = 0;
}

static char *dm_version_info(void) //(char *version_string)
{
	char path_info[42];
	sprintf(path_info, "%s", __FILE__); // dm_version_display(path_info);
	printf("\r\n\r\n\r\n");
	printf("[%s]\r\n\r\n", "POWER-ON");

	do
	{
		char ver_buf[12];
		ver_string_get(ver_buf, path_info);
		sprintf(version_string, DRIVER_VERSION, ver_buf);
	} while (0);
	printf("[DRIVER ver] %s\r\n\r\n", version_string);
	return version_string;
}

/*
 * Call as dump_data(NULL, 0); to reset
 * 'link_log_reset_allow_num_dump_data'
 * NOT used as this way, in current scope. 
 */
#define MAX_RX_DUMP_DATA_LOG_ENTRIES			2 //1

static void dump_data(uint8_t *packet_data, uint16_t packet_len)
{
	static int link_log_reset_allow_num_dump_data = 0;
	
	if (!packet_data && !packet_len) {
		link_log_reset_allow_num_dump_data = 0;
		return;
	}

	if (link_log_reset_allow_num_dump_data >= MAX_RX_DUMP_DATA_LOG_ENTRIES)
		return;
			
	uint16_t i, j, rowsize = 32;
	int splen; //index of start row
	uint16_t rlen; //remain/row length
	char line[120];

	for (i = 0; i < packet_len; i += rlen) {
		rlen =  packet_len - i;
		if (rlen >= rowsize) rlen = rowsize;

		splen = 0;
		splen += sprintf(line + splen, " %3u", i);
		for (j = 0; j < rlen; j++) {
			if (!(j % 8)) splen += sprintf(line + splen, " ");
			if (!(j % 16)) splen += sprintf(line + splen, " ");
			splen += sprintf(line + splen, " %02x", packet_data[i + j]);
		}
		printf("%s\r\n", line);
	}
	link_log_reset_allow_num_dump_data++; //.
}

#ifdef __cplusplus
}
#endif

#endif //__DM9051_H
