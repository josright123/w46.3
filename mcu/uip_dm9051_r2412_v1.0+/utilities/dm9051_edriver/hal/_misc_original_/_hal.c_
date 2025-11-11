/**
 *******************************************************************************
 * @file    hal.c
 * @brief   Hardware Abstraction Layer for DM9051 Ethernet Controller
 * 
 * @details This file provides functions for SPI initialization and communication with
 *          the DM9051 Ethernet controller.
 * 
 * @version 1.0.0
 * @author  Joseph CHANG
 * @copyright (c) 2023-2025 Davicom Semiconductor, Inc.
 * @date    2024-11-10
 *******************************************************************************
 */

#include "control/drv_control/conf_core.h"
#include "control/drv_control/dm9051_drv_debug.h"

/* Hardware Interface Functions */
uint8_t   cspi_read_reg(uint8_t reg);
void      cspi_write_reg(uint8_t reg, uint8_t val);
void      ctick_delay_us(uint32_t nus);

// Additional Functions for Register Operations
void cspi_read_regs_long(uint8_t reg, uint8_t *buf, uint16_t len);
void cspi_read_regs_each(uint8_t reg, uint8_t *buf, uint16_t len);

void cspi_read_regs(uint8_t reg, uint8_t *buf, uint16_t len, csmode_t csmode) {
    if (csmode == CS_LONG) {
        cspi_read_regs_long(reg, buf, len);
    } else { // CS_EACH
        cspi_read_regs_each(reg, buf, len);
    }
}

void cspi_write_regs(uint8_t reg, const uint8_t *buf, uint16_t len)
{
	uint16_t i;
    for (i = 0; i < len; i++, reg++) {
        cspi_write_reg(reg, buf[i]);
    }
}

/*******************************************************************************
 * PHY Access Functions
 ******************************************************************************/
uint16_t cspi_phy_read(uint16_t uReg)
{
    int w = 0;
    uint16_t uData;

    cspi_write_reg(DM9051_EPAR, DM9051_PHY | uReg);
    cspi_write_reg(DM9051_EPCR, 0xc);
    ctick_delay_us(1);
    
    while (cspi_read_reg(DM9051_EPCR) & 0x1) {
        ctick_delay_us(1);
        if (++w >= 500) break; // Timeout
    }

    cspi_write_reg(DM9051_EPCR, 0x0);
    uData = (cspi_read_reg(DM9051_EPDRH) << 8) | cspi_read_reg(DM9051_EPDRL);
    return uData;
}

void cspi_phy_write(uint16_t reg, uint16_t value)
{
    int w = 0;

    cspi_write_reg(DM9051_EPAR, DM9051_PHY | reg);
    cspi_write_reg(DM9051_EPDRL, (value & 0xff));
    cspi_write_reg(DM9051_EPDRH, ((value >> 8) & 0xff));
    cspi_write_reg(DM9051_EPCR, 0xa);
    ctick_delay_us(1);
    
    while (cspi_read_reg(DM9051_EPCR) & 0x1) {
        ctick_delay_us(1);
        if (++w >= 500) break; // Timeout
    }

    cspi_write_reg(DM9051_EPCR, 0x0);
}

// TX Request Function
void cspi_tx_req(void) {
    cspi_write_reg(DM9051_TCR, TCR_TXREQ); // Cleared after TX complete
}

// Delay Function
void ctick_delay_us(uint32_t nus)
{
    uint32_t start = dm9051_boards_heartbeat_now();
    while ((dm9051_boards_heartbeat_now() - start) < ((nus + 999) / 1000));
}

void ctick_delay_ms(uint16_t nms)
{
    uint32_t start = dm9051_boards_heartbeat_now();
    while ((dm9051_boards_heartbeat_now() - start) < nms);
}
