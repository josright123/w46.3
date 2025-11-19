/**
 *******************************************************************************
 * @file    dm9051a_ptp.c
 * @brief   DM9051A PTP (Precision Time Protocol) Implementation for AT32F403A
 *
 * @details This file implements PTP timestamp functions for the DM9051A
 *          Ethernet controller with IEEE 1588-2008 support.
 *
 *          Features:
 *          - Hardware timestamp capture for TX/RX packets
 *          - PTP clock rate adjustment and frequency correction
 *          - Time synchronization with nanosecond precision
 *          - PTP packet timestamping support
 *
 * @version 1.6.1
 * @author  Joseph CHANG / yicheng55
 * @copyright (c) 2023-2026 Davicom Semiconductor, Inc.
 * @date    2025-06-29
 *******************************************************************************
 */
#include <stdio.h>
#include <string.h>
#include "ethernetif.h"
#include "lwip/prot/ethernet.h"
#include "core/dm9051_constants.h"
#include "../../dm9051_u2510_if/if.h" //"all, as main.c including"
#include "dm9051a_ptp.h"
#define delay_us       ctick_delay_us
#define delay_ms       ctick_delay_ms

#include "ptpd.h" //#include "ptpd_v51.h" //TEMP
//#include "developer_conf.h"


//#define ptpClock_v51 ptpClock
//extern PtpClock ptpClock;

#if LWIP_PTP
int32_t v51_getPtpClockRate(int64_t *ptrRateValue);

/* DM9051A PTP Constants */
#define V51_ADJ_FREQ_BASE_ADDEND     171.7987 /* Base addend for frequency adjustment */
#define V51_ADJ_FREQ_BASE_ADDEND_FT  171.7987
#define V51_ADJ_FREQ_BASE_ADDEND_Q16 11259106 /* Q16 format: 171.7987 * 2^16 */
//#define ADJ_FREQ_MAX                 5120000  /* Maximum adjustment value */
#define MAX_RETRY_COUNT              3        /* Maximum retry for register read */

/* PTP Register Control Values */
#define ADJUST_SLOWER_CTRL           0x60       /* Control value for slower adjustment */
#define ADJUST_FASTER_CTRL           0x20       /* Control value for faster adjustment */
#define MAX_ADJUSTMENT               0xEFFFFFFF /* Maximum adjustment value */

/* Global Variables */
uint8_t        g_time_buf[8]; /* Time buffer for PTP operations */

/* External function prototypes (from DM9051 driver) */
//extern uint8_t hal_read_reg(uint8_t reg);
//extern void    hal_write_reg(uint8_t reg, uint8_t val);
//extern void    hal_read_mem(uint8_t *buf, uint16_t len);
//extern void    hal_write_mem(uint8_t *buf, uint16_t len);
//extern void    ctick_delay_us(uint32_t nus);
//extern void    ctick_delay_ms(uint16_t nms);

/* Function aliases for compatibility */
//#define cspi_read_reg  hal_read_reg
//#define cspi_write_reg hal_write_reg
//#define cspi_read_mem  hal_read_mem
//#define cspi_write_mem hal_write_mem
//#define delay_us       ctick_delay_us
//#define delay_ms       ctick_delay_ms

/**
 * @brief  Convert ppb to rate counter value for 25MHz clock, 32-bit counter
 * @param  delta_ppb: Frequency correction in ppb (parts per billion)
 * @retval Rate counter step value (signed 64-bit integer)
 */
//int64_t v51_ppb_to_rate(int32_t delta_ppb)
//{
//  const uint64_t f_clk_hz    = 25000000;       /* 25 MHz fixed clock */
//  const uint8_t  n_bits      = 32;             /* 32-bit counter */
//  const uint64_t two_power_n = 1ULL << n_bits; /* 2^32 */

//  /* Calculate numerator: delta_ppb * 2^32 */
//  int64_t numerator = (int64_t)delta_ppb * (int64_t)two_power_n;

//  /* Calculate R with rounding */
//  int64_t r = (numerator + (delta_ppb >= 0 ? (int64_t)(f_clk_hz / 2) : -(int64_t)(f_clk_hz / 2))) / (int64_t)f_clk_hz;

//  printf("ppb_to_rate: ppb:%d, numerator:%lld, rate:%lld\n", delta_ppb, numerator, r);
//  return r;
//}

/**
 * @brief  Set TX timestamp register control
 * @param  val: Control value for TX timestamp capture
 * @retval none
 */
//void v51_ptptime_set_tx_timestamp_reg(uint8_t val) { HAL_write_reg(0x02, val); }

/**
 * @brief  Initialize DM9051A PTP functionality
 * @param  sel: PTP mode selection (0: disable, 1: enable)
 * @retval none
 */
const uint8_t *dm9051a_init_ptp(const uint8_t *adr)
{
  adr = dm9051_init(adr); //dm9051dev.init(adr); //dm9051_init(adr);
  if (!adr) {
	printf("[edrv.init].error\r\n\r\n");
	return NULL;
  }
  //if (adr) {
  struct ptptime_t ts;
  int64_t rate;

  /* PTP restart sequence */
  HAL_write_reg(0x60, 0x01); /* PTP restart bit */
  delay_ms(1);
  HAL_write_reg(0x60, 0x00);

  /* Enable PTP functionality */
  HAL_write_reg(0x61, 0x01); /* PTP Enable */

  /* Disable TX timestamp capture initially */
  HAL_write_reg(0x02, 0x00); /* Disable TX PTP timestamp */

  /* Master/Slave Mode & 1588 Version Register */
  HAL_write_reg(0x64, 0x12); /* RX_EN=0x10 | multicast=0x02 */

  /* TX One Step configuration */
  HAL_write_reg(0x63, 0x00); /* TX One Step disabled */

  /* 1-step sync packet configuration */
  HAL_write_reg(0x65, 0x4E); /* Default 1-step sync packet */
  HAL_write_reg(0x66, 0x3C); /* Default 1-step sync packet */

  /* Set initial time */
  ts.tv_sec  = 2025030616; /* Default time in seconds */
  ts.tv_nsec = 0;
  dm9051_ptptime_settime(&ts); //v51_ptptime_settime

  /* Read back and display initial time */
  dm9051_ptptime_gettime(&ts); //v51_ptptime_gettime
  printf("****** v51_ptptime_gettime: %u sec %u ns\r\n", ts.tv_sec, ts.tv_nsec);
//  printf("Init PTP time: %u.%09u\r\n", ts.tv_sec, ts.tv_nsec);

  /* Get initial rate */
  v51_getPtpClockRate(&rate);
  //}
  return adr;
}

//#include "ptpd.h" //#include "ptpd_v51.h" //TEMP

void v51_ptptime_add_offset(struct ptptime_t *timestamp)
{
  int i;
  uint8_t time_buf[8];

  /* Convert to timestamp format (nanoseconds first, then seconds) */
  time_buf[0] = (uint8_t)(timestamp->tv_nsec & 0x000000FF);
  time_buf[1] = (uint8_t)((timestamp->tv_nsec & 0x0000FF00) >> 8);
  time_buf[2] = (uint8_t)((timestamp->tv_nsec & 0x00FF0000) >> 16);
  time_buf[3] = (uint8_t)((timestamp->tv_nsec & 0xFF000000) >> 24);
  time_buf[4] = (uint8_t)(timestamp->tv_sec & 0x000000FF);
  time_buf[5] = (uint8_t)((timestamp->tv_sec & 0x0000FF00) >> 8);
  time_buf[6] = (uint8_t)((timestamp->tv_sec & 0x00FF0000) >> 16);
  time_buf[7] = (uint8_t)((timestamp->tv_sec & 0xFF000000) >> 24);

  /* Reset index and write time data */
  HAL_write_reg(0x61, 0x80);

  /* Write 8 bytes of time data */
  for(i = 0; i < 8; i++)
    HAL_write_reg(0x68, time_buf[i]);

  /* Apply time setting */
  HAL_write_reg(0x61, 0x10);         // 0x10
//HAL_write_reg(0x61, 0x11);         // 0x10 | 0x01
}

void dm9051_ptptime_gettime(struct ptptime_t * timestamp) {
//  timestamp->tv_nsec = 0; //emac_ptpsubsecond2nanosecond(EMAC_PTP->tsl);
//  timestamp->tv_sec = 0; //EMAC_PTP->tsh;
  int i;

  /* Setup register to read PTP clock time */
  HAL_write_reg(0x61, 0x84); /* Read PTP Clock Time */

  /* Read 8 bytes of time data */
  for (i = 0; i < 8; i++)
  {
    g_time_buf[i] = HAL_read_reg(0x68);
  }

  /* Convert to timestamp format (nanoseconds first, then seconds) */
  timestamp->tv_nsec = (uint32_t)g_time_buf[0] | (uint32_t)g_time_buf[1] << 8 | (uint32_t)g_time_buf[2] << 16 |
                       (uint32_t)g_time_buf[3] << 24;
  timestamp->tv_sec = (uint32_t)g_time_buf[4] | (uint32_t)g_time_buf[5] << 8 | (uint32_t)g_time_buf[6] << 16 |
                      (uint32_t)g_time_buf[7] << 24;
}

static int64_t last_rate = 0; /* Last cumulative adjustment value */
void dm9051_ptptime_settime(struct ptptime_t * timestamp)
{
  int     i;
  uint8_t time_buf[8];

  /* Convert timestamp to byte array (nanoseconds first, then seconds) */
  time_buf[0] = (uint8_t)(timestamp->tv_nsec & 0x000000FF);
  time_buf[1] = (uint8_t)((timestamp->tv_nsec & 0x0000FF00) >> 8);
  time_buf[2] = (uint8_t)((timestamp->tv_nsec & 0x00FF0000) >> 16);
  time_buf[3] = (uint8_t)((timestamp->tv_nsec & 0xFF000000) >> 24);
  time_buf[4] = (uint8_t)(timestamp->tv_sec & 0x000000FF);
  time_buf[5] = (uint8_t)((timestamp->tv_sec & 0x0000FF00) >> 8);
  time_buf[6] = (uint8_t)((timestamp->tv_sec & 0x00FF0000) >> 16);
  time_buf[7] = (uint8_t)((timestamp->tv_sec & 0xFF000000) >> 24);

  /* PTP restart sequence */
  HAL_write_reg(0x60, 0x01); /* PTP restart bit */
  delay_us(2);
  HAL_write_reg(0x60, 0x00);
  last_rate = 0; /* Reset accumulated adjustment */

  /* Reset index and write time data */
  HAL_write_reg(0x61, 0x80); /* Reset register index */

  /* Write 8 bytes of time data */
  for (i = 0; i < 8; i++)
  {
    HAL_write_reg(0x68, time_buf[i]);
  }

  /* Apply time setting */
  HAL_write_reg(0x61, 0x09); /* Write PTP Clock Time | PTP Enable */

  printf("****** v51_ptptime_settime: %u sec %u ns\r\n", timestamp->tv_sec, timestamp->tv_nsec);
}

/**
 * @brief  Adjust PTP frequency
 * @param  adjustment: Adjustment value
 * @param  direction: 0 = faster, 1 = slower
 * @retval none
 */
void v51_adjust_ptp_frequency(uint32_t adjustment, int8_t direction)
{
  uint8_t rate_bytes[4] = {0};
  uint8_t control_value;
  uint8_t reg_index[4]      = {0};
  uint8_t index_expected[4] = {0x10, 0x20, 0x30, 0x40};
  uint8_t valid_indices     = 1;

  /* Parameter validation */
  if (adjustment > MAX_ADJUSTMENT)
  {
    printf("[PTP] Error: Adjustment value 0x%08lX exceeds maximum allowed\n", adjustment);
    return;
  }

  /* Convert 32-bit adjustment to byte array (little-endian) */
  rate_bytes[0] = (uint8_t)(adjustment);
  rate_bytes[1] = (uint8_t)(adjustment >> 8);
  rate_bytes[2] = (uint8_t)(adjustment >> 16);
  rate_bytes[3] = (uint8_t)(adjustment >> 24);

  /* Reset register index */
  HAL_write_reg(0x61, 0x80);

  /* Write and verify each byte */
  for (uint8_t i = 0; i < 4; i++)
  {
    HAL_write_reg(0x68, rate_bytes[i]);
    reg_index[i] = HAL_read_reg(0x69);

    /* Check if index matches expected value */
    if (reg_index[i] != index_expected[i])
    {
      valid_indices = 0;
    }
  }

  /* Apply adjustment only if indices are valid */
  if (valid_indices)
  {
    control_value = (direction == 1) ? ADJUST_SLOWER_CTRL : ADJUST_FASTER_CTRL;
    HAL_write_reg(0x61, control_value);
  }
  else
  {
    printf("[PTP] Rate adjustment skipped due to invalid indices\n");
  }
}

#define v51_ptp_time_adj_freq dm9051_ptptime_adjfreq
//void dm9051_ptptime_adjfreq(int32_t Adj)
//{
//    /* TODO: Implement frequency adjustment using DM9051A hardware registers */
//    /* This function should be implemented based on DM9051A PTP specifications */
//    printf("dm9051_ptptime_adjfreq: adj=%d ppb (not yet implemented)\r\n", Adj);
//    
//    /* Placeholder implementation - should be replaced with actual hardware control */
//    /* Example implementation would involve:
//     * 1. Convert ppb to hardware-specific rate adjustment value
//     * 2. Write adjustment value to DM9051A PTP rate control registers
//     * 3. Apply the adjustment to the hardware clock
//     */
//}

/**
 * @brief  Adjust PTP frequency based on adj value
 * @param  adj: Adjustment value in ppb
 * @retval none
 */
void v51_ptp_time_adj_freq(int32_t adj)
{
  // dm9051a ptp clock 25M hz 計算參數: 2^32*40/10^9 = 171.79869184 rate = 1ppb.
  // #define V51_ADJ_FREQ_BASE_ADDEND           171.7987

#if 0
  static int32_t last_adj     = 0;              // 保存上一次的調整量
  int32_t        original_adj = adj;            // 保存原始 adj 值用於日誌輸出
  int32_t        delta_adj    = adj - last_adj; // 計算此次與上次調整量的差值
#endif
  // 以參數直接換算 ppb_to_rate
  int64_t signed_addend = (int64_t)(adj * V51_ADJ_FREQ_BASE_ADDEND);
  // 使用放大倍率計算調整值
  // int64_t signed_addend = calculate_scaled_addend(adj);
#if 0
  // 使用 Q16 定點數計算
  int64_t signed_addend_q16 = ((int64_t)adj * V51_ADJ_FREQ_BASE_ADDEND_Q16) >> 16;
  // 使用 Float 計算
  int64_t signed_addend_ft = ((int64_t)adj * V51_ADJ_FREQ_BASE_ADDEND_FT);
#endif
  int64_t delta_rate = signed_addend - last_rate;

  // 決定最終調整方向與值
  uint32_t adjust_value;
  int8_t   adjust_direction;

  if (delta_rate < 0)
  {
    adjust_direction = 1; // 慢下來
    adjust_value     = (uint32_t)(-delta_rate);
  }
  else
  {
    adjust_direction = 0; // 加快
    adjust_value     = (uint32_t)delta_rate;
  }

#if 0
  last_adj = adj;
#endif

  // 呼叫硬體調整函數
  v51_adjust_ptp_frequency(adjust_value, adjust_direction);

#if 0
  // 增強的日誌輸出 - 統一使用 delta_rate 判斷實際調整方向
  const char *status_msg;
  if (delta_rate > 0)
    status_msg = "to be faster";
  else if (delta_rate < 0)
    status_msg = "to be slower";
  else
    status_msg = "no adjustment";

  printf("dm9051a rate adjust = %u(0x%08X) %s! (delta_rate:%lld, last_rate:%lld)\r\n", adjust_value, adjust_value,
         status_msg, delta_rate, last_rate);
  // printf("Original adj: %d ppb, delta_adj: %d ppb, signed_addend: %lld\r\n",
  //     original_adj, delta_adj, signed_addend);
  printf("Original adj: %d ppb, delta_adj: %d ppb, signed_addend: %lld, signed_addend_q16: %lld, signed_addend_ft: "
         "%lld\r\n",
         original_adj, delta_adj, signed_addend, signed_addend_q16, signed_addend_ft);
#endif

  // 更新累積調整值
  last_rate = signed_addend;
}

void dm9051_ptptime_updateoffset(struct ptptime_t * timeoffset)
{
    struct ptptime_t current_time;
    struct ptptime_t new_time;
    
    /* Get current time */
    dm9051_ptptime_gettime(&current_time); //v51_ptptime_gettime
    
    /* Calculate new time by adding offset */
    new_time.tv_sec = current_time.tv_sec + timeoffset->tv_sec;
    new_time.tv_nsec = current_time.tv_nsec + timeoffset->tv_nsec;
    
    /* Handle nanosecond overflow/underflow */
    if (new_time.tv_nsec >= 1000000000) {
        new_time.tv_sec += 1;
        new_time.tv_nsec -= 1000000000;
    } else if (new_time.tv_nsec < 0) {
        new_time.tv_sec -= 1;
        new_time.tv_nsec += 1000000000;
    }
    
    /* Set the new time */
    dm9051_ptptime_settime(&new_time); //v51_ptptime_settime
    
    printf("dm9051_ptptime_updateoffset: offset %ds %dns applied\r\n", 
           timeoffset->tv_sec, timeoffset->tv_nsec);
}

#if 0
/**
 * @brief  Get current PTP time from DM9051A
 * @param  timestamp: Pointer to timestamp structure
 * @retval none
 */
void v51_ptptime_gettime(struct ptptime_t *timestamp)
{
  int i;

  /* Setup register to read PTP clock time */
  HAL_write_reg(0x61, 0x84); /* Read PTP Clock Time */

  /* Read 8 bytes of time data */
  for (i = 0; i < 8; i++)
  {
    g_time_buf[i] = HAL_read_reg(0x68);
  }

  /* Convert to timestamp format (nanoseconds first, then seconds) */
  timestamp->tv_nsec = (uint32_t)g_time_buf[0] | (uint32_t)g_time_buf[1] << 8 | (uint32_t)g_time_buf[2] << 16 |
                       (uint32_t)g_time_buf[3] << 24;
  timestamp->tv_sec = (uint32_t)g_time_buf[4] | (uint32_t)g_time_buf[5] << 8 | (uint32_t)g_time_buf[6] << 16 |
                      (uint32_t)g_time_buf[7] << 24;
}

/**
 * @brief  Set PTP time to DM9051A
 * @param  timestamp: Pointer to timestamp structure
 * @retval none
 */
void v51_ptptime_settime(struct ptptime_t *timestamp)
{
  int     i;
  uint8_t time_buf[8];

  /* Convert timestamp to byte array (nanoseconds first, then seconds) */
  time_buf[0] = (uint8_t)(timestamp->tv_nsec & 0x000000FF);
  time_buf[1] = (uint8_t)((timestamp->tv_nsec & 0x0000FF00) >> 8);
  time_buf[2] = (uint8_t)((timestamp->tv_nsec & 0x00FF0000) >> 16);
  time_buf[3] = (uint8_t)((timestamp->tv_nsec & 0xFF000000) >> 24);
  time_buf[4] = (uint8_t)(timestamp->tv_sec & 0x000000FF);
  time_buf[5] = (uint8_t)((timestamp->tv_sec & 0x0000FF00) >> 8);
  time_buf[6] = (uint8_t)((timestamp->tv_sec & 0x00FF0000) >> 16);
  time_buf[7] = (uint8_t)((timestamp->tv_sec & 0xFF000000) >> 24);

  /* PTP restart sequence */
  HAL_write_reg(0x60, 0x01); /* PTP restart bit */
  delay_us(2);
  HAL_write_reg(0x60, 0x00);
  last_rate = 0; /* Reset accumulated adjustment */

  /* Reset index and write time data */
  HAL_write_reg(0x61, 0x80); /* Reset register index */

  /* Write 8 bytes of time data */
  for (i = 0; i < 8; i++)
  {
    HAL_write_reg(0x68, time_buf[i]);
  }

  /* Apply time setting */
  HAL_write_reg(0x61, 0x09); /* Write PTP Clock Time | PTP Enable */

  printf("\r\n");
  printf("****** _v51_ptptime_settime: %u sec %u ns\r\n", timestamp->tv_sec, timestamp->tv_nsec);
}
#endif

/**
 * @brief  Adjust PTP frequency
 * @param  adjustment: Adjustment value
 * @param  direction: 0 = faster, 1 = slower
 * @retval none
 */
//void v51_adjustPtpFrequency(uint32_t adjustment, int8_t direction)
//{
//  uint8_t rateBytes[4] = {0};
//  uint8_t controlValue;
//  uint8_t reg_index[4]      = {0};
//  uint8_t index_expected[4] = {0x10, 0x20, 0x30, 0x40};
//  uint8_t valid_indices     = 1;

//  /* Parameter validation */
//  if (adjustment > MAX_ADJUSTMENT)
//  {
//    printf("[PTP] Error: Adjustment value 0x%08lX exceeds maximum allowed\n", adjustment);
//    return;
//  }

//  /* Convert 32-bit adjustment to byte array (little-endian) */
//  rateBytes[0] = (uint8_t)(adjustment);
//  rateBytes[1] = (uint8_t)(adjustment >> 8);
//  rateBytes[2] = (uint8_t)(adjustment >> 16);
//  rateBytes[3] = (uint8_t)(adjustment >> 24);

//  /* Reset register index */
//  HAL_write_reg(0x61, 0x80);

//  /* Write and verify each byte */
//  for (uint8_t i = 0; i < 4; i++)
//  {
//    HAL_write_reg(0x68, rateBytes[i]);
//    reg_index[i] = HAL_read_reg(0x69);

//    /* Check if index matches expected value */
//    if (reg_index[i] != index_expected[i])
//    {
//      valid_indices = 0;
//    }
//  }

//  /* Apply adjustment only if indices are valid */
//  if (valid_indices)
//  {
//    controlValue = (direction == 1) ? ADJUST_SLOWER_CTRL : ADJUST_FASTER_CTRL;
//    HAL_write_reg(0x61, controlValue);
//  }
//  else
//  {
//    printf("[PTP] Rate adjustment skipped due to invalid indices\n");
//  }
//}

/**
 * @brief  Get TX timestamp from DM9051A
 * @param  pTimeStamp: Pointer to timestamp structure
 * @retval none
 */
void impl_dm9051_get_tx_timestamp(struct ptptime_t *pTimeStamp)
{
  uint8_t timeStampBuf[8] = {0};
  int     i;

  /* Reset timestamp read index */
  HAL_write_reg(0x61, 0x80);

  /* Set TX timestamp read mode */
  HAL_write_reg(0x62, 0x01);

  /* Read 8 bytes of timestamp data */
  for (i = 0; i < 8; i++)
  {
    timeStampBuf[i] = HAL_read_reg(0x68);
  }

  /* Convert to standard timestamp format (nanoseconds first, then seconds) */
  pTimeStamp->tv_nsec = (uint32_t)timeStampBuf[0] | (uint32_t)timeStampBuf[1] << 8 | (uint32_t)timeStampBuf[2] << 16 |
                        (uint32_t)timeStampBuf[3] << 24;

  pTimeStamp->tv_sec = (uint32_t)timeStampBuf[4] | (uint32_t)timeStampBuf[5] << 8 | (uint32_t)timeStampBuf[6] << 16 |
                       (uint32_t)timeStampBuf[7] << 24;
}

/**
 * @brief  Set last rate value for PTP clock adjustment
 * @param  rate: New rate value to set
 * @retval none
 */
//void v51_set_last_rate(int64_t rate)
//{
//  int64_t getrate;

//  /* Only adjust if rate has changed */
//  if (rate != last_rate)
//  {
//    /* Calculate difference between current and new rate */
//    int64_t delta = last_rate - rate;

//    /* Determine adjustment direction and absolute value */
//    uint32_t adjust_value;
//    int8_t   adjust_direction;

//    if (delta < 0)
//    {
//      adjust_direction = 1; /* Slow down clock */
//      adjust_value     = (uint32_t)(-delta);
//    }
//    else
//    {
//      adjust_direction = 0; /* Speed up clock */
//      adjust_value     = (uint32_t)delta;
//    }

//    /* Apply rate adjustment to hardware */
//    v51_adjustPtpFrequency(adjust_value, adjust_direction);

//    /* Log the adjustment details */
//    printf("Reset 9051a rate: %lld (delta: %+lld)\n", last_rate, delta);
//    v51_getPtpClockRate(&getrate);
//    printf("---  v51_getrate: 0x%08llX (%lld)\n", getrate, getrate);
//  }
//}

/**
 * @brief  Adjust PTP frequency based on adj value
 * @param  adj: Adjustment value in ppb
 * @retval none
 */
//void v51_ptptime_adjfreq(int32_t adj)
//{
//  static int32_t last_adj     = 0;   /* Last adjustment value */
//  int32_t        original_adj = adj; /* Original adj for logging */

//  /* Calculate signed addend using base addend */
//  int64_t signed_addend = (int64_t)(adj * V51_ADJ_FREQ_BASE_ADDEND);

//  /* Calculate Q16 fixed-point version */
//  int64_t signed_addend_Q16 = ((int64_t)adj * V51_ADJ_FREQ_BASE_ADDEND_Q16) >> 16;

//  int64_t delta_rate = signed_addend - last_rate;

//  /* Determine final adjustment direction and value */
//  uint32_t adjust_value;
//  int8_t   adjust_direction;

//  if (delta_rate < 0)
//  {
//    adjust_direction = 1; /* Slow down */
//    adjust_value     = (uint32_t)(-delta_rate);
//  }
//  else
//  {
//    adjust_direction = 0; /* Speed up */
//    adjust_value     = (uint32_t)delta_rate;
//  }

//  last_adj = adj;

//  /* Call hardware adjustment function */
//  v51_adjustPtpFrequency(adjust_value, adjust_direction);

//  /* Enhanced logging output */
//  const char *status_msg;
//  if (delta_rate > 0)
//    status_msg = "to be faster";
//  else if (delta_rate < 0)
//    status_msg = "to be slower";
//  else
//    status_msg = "no adjustment";

//  printf("dm9051a rate adjust = %u(0x%08X) %s! (delta_rate:%lld, last_rate:%lld)\r\n", adjust_value, adjust_value,
//         status_msg, delta_rate, last_rate);
//  printf("Original adj: %d ppb, signed_addend: %lld, signed_addend_Q16: %lld\r\n", original_adj, signed_addend,
//         signed_addend_Q16);

//  /* Update cumulative adjustment value */
//  last_rate = signed_addend;
//}

/**
 * @brief  Helper function for rate debug logging
 * @param  buffer: Rate buffer data
 * @retval none
 */
static void logRateDebugInfo(const uint8_t *buffer)
{
  /* Extract rate from buffer (bytes 0-3, little-endian) */
  uint32_t rate =
      (uint32_t)buffer[3] << 24 | (uint32_t)buffer[2] << 16 | (uint32_t)buffer[1] << 8 | (uint32_t)buffer[0];

  /* Extract sign from buffer (byte 4) */
  uint8_t sign = buffer[4] & 0x01;

  /* Determine sign character and direction */
  char        signChar  = ((sign & 0x01) == 0) ? '+' : '-';
  const char *direction = (sign == 0) ? "faster" : "slower";

  /* Print formatted rate information */
//  printf("dm9051a Rate: %c0x%08X (%c%ld) - Clock adjusted to be %s\r\n", signChar, rate, signChar, (long)rate, direction);
  printf("****** dm9051a Rate: %c0x%08X (%c%ld) - Clock adjusted to be %s\r\n", signChar, rate, signChar, (long)rate, direction);
}

/**
 * @brief  Reset and setup PTP clock read mode
 * @retval none
 */
static void reset_ptp_clock_read_mode(void)
{
  HAL_write_reg(0x69, 0x01); /* Enable rate read mode */
  HAL_write_reg(0x61, 0x80); /* Reset register index */
}

/**
 * @brief  Validate register index value
 * @param  index: Current index value
 * @param  position: Index position (0-3)
 * @retval 1 if index is correct, 0 if error
 */
static uint8_t validate_register_index(uint8_t index, uint8_t position)
{
  const uint8_t expected_indices[] = {0x10, 0x20, 0x30, 0x40};
  return (index == expected_indices[position]);
}

/**
 * @brief  Read PTP clock rate value
 * @param  rateBuffer: Buffer to store rate value
 * @param  reg_index: Buffer to store register indices
 * @retval 1 if read successful, 0 if failed
 */
static uint8_t read_ptp_clock_rate(uint8_t *rateBuffer, uint8_t *reg_index)
{
  uint8_t retry_count = 0;
  uint8_t success     = 0;

  while (retry_count < MAX_RETRY_COUNT && !success)
  {
	uint8_t i;

    reset_ptp_clock_read_mode();
    success = 1;

    /* Read 8 bytes of data */
    for (i = 0; i < 8 && success; i++)
    {
      rateBuffer[i] = HAL_read_reg(0x68);
      reg_index[i]  = HAL_read_reg(0x69);

      /* Check only first 4 index values */
      if (i < 4 && !validate_register_index(reg_index[i], i))
      {
        success = 0;
        retry_count++;
        printf("Invalid index at position %d: 0x%02X (retry %d)\n", i, reg_index[i], retry_count);
        break;
      }
    }
  }

  return success;
}

/**
 * @brief  Get current PTP clock rate adjustment value
 * @param  ptrRateValue: Pointer to store rate value
 * @retval 0 on success, negative on error
 */
int32_t v51_getPtpClockRate(int64_t *ptrRateValue)
{
  uint8_t       rateBuffer[8]     = {0};
  uint8_t       reg_index[8]      = {0};
  uint32_t      absoluteRate      = 0;
  const int32_t success           = 0;
  const int32_t invalidParamError = -1;
  const int32_t readError         = -2;

  /* Validate input parameter */
  if (!ptrRateValue)
  {
    return invalidParamError;
  }

  /* Attempt to read clock rate */
  if (!read_ptp_clock_rate(rateBuffer, reg_index))
  {
    printf("Failed to read PTP clock rate after %d attempts\n", MAX_RETRY_COUNT);
    return readError;
  }

  /* Combine 32-bit rate value (little-endian format) */
  absoluteRate = (uint32_t)rateBuffer[3] << 24 | (uint32_t)rateBuffer[2] << 16 | (uint32_t)rateBuffer[1] << 8 |
                 (uint32_t)rateBuffer[0];

  /* Set final value based on sign bit */
  *ptrRateValue = ((rateBuffer[4] & 0x01) == 0) ? (int64_t)absoluteRate : -(int64_t)absoluteRate;

  /* Output debug information */
  logRateDebugInfo(rateBuffer);

  return success;
}

/* match library
 *  is_packet_match(type, DELAY_REQ)
 */

#define ETH_HLEN	14		/* Total octets in header.	 */

#define is_packet_match(buf, matchto) (valid_ptp_message_type(buf) && get_ptp_message_type005(buf) == matchto)

#define is_packet_match_onestep(buf, matchto) \
		(is_packet_match(buf, matchto) && DEFAULT_TWO_STEP_FLAG == FALSE)

int is_ptp_sync_packet(u8_t msgtype)
{
	return (msgtype == SYNC) ? 1 : 0;
}

int is_ptp_delayreq_packet(u8_t msgtype)
{
	return (msgtype == DELAY_REQ) ? 1 : 0;
}

int is_ptp_delayresp_packet(u8_t msgtype)
{
	return (msgtype == DELAY_RESP) ? 1 : 0;
}

int is_ptp_pdelayreq_packet(u8_t msgtype)
{
	return (msgtype == PDELAY_REQ) ? 1 : 0;
}

int is_ptp_pdelayresp_packet(u8_t msgtype)
{
	return (msgtype == PDELAY_RESP) ? 1 : 0;
}

u16_t get_ptp_src_port(uint8_t *buf)
{
	buf += ETH_HLEN;
	struct udp_hdr *udp = (struct udp_hdr *)(buf + sizeof(struct ip_hdr));
	return PP_HTONS(udp->src);
}

u16_t get_ptp_dest_port(uint8_t *buf)
{
	buf += ETH_HLEN;
	struct udp_hdr *udp = (struct udp_hdr *)(buf + sizeof(struct ip_hdr));
	return PP_HTONS(udp->dest);
}

/* match library base
 */

#define ETH_HLEN	14		/* Total octets in header.	 */
#define IPPROTO_UDP		17

int valid_ptp_message_type(uint8_t *buf)
{
	//struct eth_hdr *eth = (struct eth_hdr *)buf;
	buf += ETH_HLEN;
	struct ip_hdr *ip = (struct ip_hdr *)buf;
	struct udp_hdr *udp = (struct udp_hdr *)(buf + sizeof(struct ip_hdr));
	/*  check
	 * (udp_proto == 17) && (udp_srcport == 319) && (udp_dstport == 319)
	 */
	if ((IPH_PROTO(ip) == IPPROTO_UDP) &&
		(PP_HTONS(udp->src) == PTP_EVENT_PORT || PP_HTONS(udp->src) == PTP_GENERAL_PORT) &&
		(PP_HTONS(udp->dest) == PTP_EVENT_PORT || PP_HTONS(udp->dest) == PTP_GENERAL_PORT)
		)
	  return 1;
	return 0;
}

u8_t get_ptp_message_type005(uint8_t *buf)
{
	MsgHeader header;
	msgUnpackHeader((const Octet *)(buf+14+20+8), &header);
	return header.messageType;
}

int is_issue_ptp_sync_one_step_emit(uint8_t *buf)
{
  if (!valid_ptp_message_type(buf))
	  return 0;

  u8_t msgtype = get_ptp_message_type005(buf);
  u16_t src_port = get_ptp_src_port(buf);
  u16_t dest_port = get_ptp_dest_port(buf);
  return (src_port == PTP_EVENT_PORT && dest_port == PTP_EVENT_PORT &&
		is_ptp_sync_packet(msgtype) &&
		DEFAULT_TWO_STEP_FLAG == FALSE)
		  /* PTP_EVENT_PORT is 319 */
		  /*|| is_ptp_delayreq_packet(msgtype)*/ ;
}

int is_issue_ptp_tstamp_tsen(uint8_t *buf)
{
  if (!valid_ptp_message_type(buf))
	  return 0;

  u8_t msgtype = get_ptp_message_type005(buf);
  return (is_ptp_sync_packet(msgtype) && DEFAULT_TWO_STEP_FLAG == TRUE) ||
		  is_ptp_delayreq_packet(msgtype) ||
		  is_ptp_pdelayreq_packet(msgtype) /*|| is_ptp_pdelayresp_packet(msgtype)*/;
}

/* is_packet_match(buf, DELAY_REQ)
 */
int is_issue_ptp_tstamp_delayreq(uint8_t *buf)
{
  if (!valid_ptp_message_type(buf))
	  return 0;

  u8_t msgtype = get_ptp_message_type005(buf);
  return is_ptp_delayreq_packet(msgtype);
}

int is_issue_ptp_tstamp_delayresp_packet(uint8_t *buf)
{
  if (!valid_ptp_message_type(buf))
	  return 0;

	u8_t msgtype = get_ptp_message_type005(buf);
	return is_ptp_delayresp_packet(msgtype);
}

void ptp_tx_tstamp_zero(uint8_t *buffer)
{
	const uint8_t *buf = (buffer+14+20+8);

    /*Sync message*/
    *(UInteger16*)(buf + 34) = 0; //flip16(originTimestamp->secondsField.msb);
    *(UInteger32*)(buf + 36) = 0; //flip32(originTimestamp->secondsField.lsb);
    *(UInteger32*)(buf + 40) = 0; //flip32(originTimestamp->nanosecondsField);
}

//uint16_t dm9051_rx_ptp(uint8_t *buff, uint8_t *ts_bff) //dm9051_rx_ptp <== impl_dm9051_rx_ptp()
//{
//  //uint8_t          rxbyte, rx_status;
//  //uint8_t          ReceiveData[4];
////  uint8_t          rxbyte;
//  uint16_t         rx_len; //, pad = 0;
//  //struct ptptime_t ts_time;
//  /* Read RX status byte */
////  rxbyte = HAL_read_reg(0x7E); /* Read ISR to check RX ready */
////  if (!(rxbyte & 0x01))
////  { /* Check RX ready bit */
////    return 0;
////  }
//  /* Read packet header - this needs to be implemented based on DM9051 specs */
//  /* For now, return standard RX function result */
//  rx_len = 0;
//  if (cspi_rx_ready()) {
//    rx_len = cspi_rx_read(buff, cspi_rx_head_ptp(x));
//  }
////  if (rx_len > 0 && ts_bff != NULL)
////  {
////    /* Extract timestamp if available */
////    /* This would need actual implementation based on DM9051A PTP specs */
////    memset(ts_bff, 0, 8); /* Clear timestamp buffer */
////  }
//  return rx_len;
//}

/**
 * @brief  Receive packet with PTP timestamp
 * @param  buff: Packet buffer
 * @param  ts_bff: Timestamp buffer (8 bytes)
 * @retval Packet length
 */
uint16_t dm9051_rx_ptp(uint8_t *buffer, uint8_t *receivedata, uint8_t *ts_bff) //dm9051_rx_ptp(uint8_t *buff, uint8_t *ts_bff)
{
  if (cspi_rx_ready())
	return cspi_rx_read(buffer, cspi_rx_head_ptp(receivedata, ts_bff));
  return 0;
}

//int ptp_rx_tstamp_process_flg = 0;
//void set_ptp_rx_tstamp_process(int val)
//{
//	ptp_rx_tstamp_process_flg = val;
//}

void dm9051_tx_ptp(uint8_t *buff, uint16_t len, struct pbuf *p)
{
//	if (!ptp_rx_tstamp_process_flg) {
//		dm9051_tx(buff, len);
//		return;
//	}

	/* Testing 2025-03-03 */
	// Fix sync packet error occurring after 3 hours of running.
	tx_pointer_timeout(500); //HAL_write_reg(0x55, 0x02); while (HAL_read_reg(0x55) & 0x02) ;
	len = cspi_tx_packet_len(len);
	if (len) {
		uint8_t tcr_wr;
		tcr_wr = ptp_tx_tstamp_parse_packet(buff, p); // previous: ptp_tx_tstamp
		cspi_tx_write(buff, len);
		tx_compl_timeout(tcr_wr, 500); //while (HAL_read_reg(DM9051_TCR) & TCR_TXREQ) ;
		ptp_tx_tstamp_pass_to(buff, p, tcr_wr); // post: ptp_tx_tstamp
//		printf(" ------------------dm9051_tx_ptp(buffer, l, p) tcr_wr %02x ----- %02x%02x%02x%02x%02x%02x len %d\r\n",
//			tcr_wr, buff[0], buff[1], buff[2], buff[3], buff[4], buff[5], len);
	}
}

#define ETH_HLEN	14		/* Total octets in header.	 */
#define ETH_P_IP	0x0800		/* Internet Protocol packet	*/
#define ETH_P_IPV6	0x86DD		/* IPv6 over bluebook		*/
#define ETH_P_LLDP  0x88CC // Layer 2 LLDP
#define IPPROTO_UDP		17
#define IPPROTO_IGMP	2
#define UDP_SSDP_PORT	1900
#define UDP_MDNS_PORT	5353
#define UDP_LLMNR_PORT	5355
#define UDP_XML_PORT	3702

char *parse_buf_ptp_packet1(uint8_t *buf, int off)
{
    if(buf[off] == 0x8b && buf[32+off] == CTRL_OTHER)
        return "Announce";
    else
	if(buf[off] == 0x80 && buf[32+off] == CTRL_SYNC)
        return "Sync";
    else
	if(buf[off] == 0x89 && buf[32+off] == CTRL_DELAY_RESP)
        return "Delay_Resp";
    else
	if(buf[off] == 0x81 && buf[32+off] == CTRL_DELAY_REQ)
        return "Delay_Req";
    else
        return "Unknown";
}

char *parse_pbuf_ptp_packet1(uint8_t *buf, int in_offset)
{
	return parse_buf_ptp_packet1(buf, in_offset);
}

char *get_ptp_header(uint8_t *p, uint16_t len, int rxts_en_packet)
{
	int printflg = 1;
	struct eth_hdr *eth = (struct eth_hdr *)p;
	u8 *ptp_hdr;
	u16 proto;
	struct ip_hdr *ip;

	// Skip Ethernet header
	p += ETH_HLEN;
	proto = eth->type;
	proto = PP_HTONS(eth->type); //ntohs(eth->type);

	// Check for Layer 2 PTP
	if (proto == PTP_ETHERTYPE) {
		return (char *) p;
	}
	
	if (proto == ETH_P_LLDP && printflg) {
		static int ll2_pnt = 2;
		if (ll2_pnt) {
			ll2_pnt--;
			printf("get_ptp_header LLDP/LL2 packet\r\n");
		}
		return NULL;
	}

	// Handle IPv4
	ip = (struct ip_hdr *)p;
	if (proto == ETH_P_IP) {
		if (IPH_PROTO(ip) == IPPROTO_UDP) {
			struct udp_hdr *udp = (struct udp_hdr *)(p + sizeof(struct ip_hdr));
			if (PP_HTONS(udp->dest) == PTP_EVENT_PORT || PP_HTONS(udp->dest) == PTP_GENERAL_PORT) {
				ptp_hdr = (u8 *)udp + sizeof(struct udp_hdr);
				return (char *) ptp_hdr;
			}
			else if (PP_HTONS(udp->dest) == UDP_SSDP_PORT && printflg) {
//				printf("get_ptp_header UDP(SSDP) packet\r\n");
			}
			else if (PP_HTONS(udp->dest) == UDP_LLMNR_PORT && printflg) {
//				printf("get_ptp_header UDP(LLMNR) packet\r\n");
			}
			else if (PP_HTONS(udp->src) == UDP_MDNS_PORT && PP_HTONS(udp->dest) == UDP_MDNS_PORT && printflg) {
//				printf("get_ptp_header UDP(MDNS.adv) packet\r\n");
			}
			else if (PP_HTONS(udp->dest) == UDP_MDNS_PORT && printflg) {
//				printf("get_ptp_header UDP(MDNS) packet\r\n");
			}
			else if (PP_HTONS(udp->dest) == UDP_XML_PORT && printflg) {
//				printf("get_ptp_header UDP(XML) packet\r\n");
			}
			/* packet 224.0.0.251 */
			else if (((ip->dest.addr >> 24) & 0xff) == 251 && ((ip->dest.addr >> 16) & 0xff) == 0 && 
				((ip->dest.addr >> 8) & 0xff) == 0 && ((ip->dest.addr >> 0) & 0xff) == 224 && printflg) {
//				printf("get_ptp_header dst 224.0.0.251 packet\r\n");
			}
			return NULL;
		}
		if (IPH_PROTO(ip) == IPPROTO_IGMP && printflg) {
//			printf("get_ptp_header IGMP/IP packet\r\n");
			return NULL;
		}
	}
	// Handle IPv6
//	else if (proto == ETH_P_IPV6) {
//		//struct ipv6hdr *ip6 = (struct ipv6hdr *)p;
//		struct ip6_hdr *ip6 = (struct ip6_hdr *)p;
//		if (ip6->nexthdr == IPPROTO_UDP) {
//			struct udp_hdr *udp = (struct udp_hdr *)(p + sizeof(struct ip6_hdr));
//			if (PP_HTONS(udp->dest) == PTP_EVENT_PORT || PP_HTONS(udp->dest) == PTP_GENERAL_PORT) {
//				ptp_hdr = (u8 *)udp + sizeof(struct udp_hdr);
//				return (char *) ptp_hdr;
//			}
//			return NULL;
//		}
//	}

	if (rxts_en_packet && printflg) {
		printf("dst.ip %d.%d.%d.%d \r\n",
			(ip->dest.addr >> 0) & 0xff,
			(ip->dest.addr >> 8) & 0xff,
			(ip->dest.addr >> 16) & 0xff,
			(ip->dest.addr >> 24) & 0xff);

		/* rare, extra 24.1.0.224 */
		if (((ip->dest.addr >> 24) & 0xff) == 224 && ((ip->dest.addr >> 16) & 0xff) == 0 && 
			((ip->dest.addr >> 8) & 0xff) == 1 && ((ip->dest.addr >> 0) & 0xff) == 24) {
			printf("=get_ptp_header dst 24.1.0.224 packet\r\n");
			return NULL;
		}
		printf("low_level_input_ (Check not a PTP packet) rx len %u dumpdata %d\r\n", len, 40);
		dump_data(eth->dest.addr, 40);
	}
	return NULL; // Not a PTP packet
}

void buffer_ts_time(uint8_t *buffer, TimeInternal *pTimeTmp)
{
    /* Extract receiveTimestamp from Delay_Resp message packet
     * Based on PTP message structure and msgUnpackDelayResp function
     *
     * Delay_Resp message structure:
     * - PTP header: 34 bytes (0-33)
     * - receiveTimestamp: starts at offset 34
     *   - secondsField.msb: 2 bytes at offset 34-35 (big-endian)
     *   - secondsField.lsb: 4 bytes at offset 36-39 (big-endian)
     *   - nanosecondsField: 4 bytes at offset 40-43 (big-endian)
     */

    /* Skip PTP header (34 bytes) and extract timestamp fields */
    uint8_t *timestamp_ptr = buffer + 34;

    /* Extract secondsField.msb (16-bit, big-endian) */
//    uint16_t seconds_msb = (uint16_t)timestamp_ptr[0] << 8 | (uint16_t)timestamp_ptr[1];

    /* Extract secondsField.lsb (32-bit, big-endian) */
    uint32_t seconds_lsb = (uint32_t)timestamp_ptr[2] << 24 |
                           (uint32_t)timestamp_ptr[3] << 16 |
                           (uint32_t)timestamp_ptr[4] << 8 |
                           (uint32_t)timestamp_ptr[5];

    /* Extract nanosecondsField (32-bit, big-endian) */
    uint32_t nanoseconds = (uint32_t)timestamp_ptr[6] << 24 |
                           (uint32_t)timestamp_ptr[7] << 16 |
                           (uint32_t)timestamp_ptr[8] << 8 |
                           (uint32_t)timestamp_ptr[9];

    /* Convert to TimeInternal format */
    /* For most practical purposes, we only use the lsb part of seconds */
    pTimeTmp->seconds = (int32_t)seconds_lsb;
    pTimeTmp->nanoseconds = (int32_t)nanoseconds;
}

struct ptptime_t delayREQ_T3, master_recv_extra_T3;

//#if (EDRIVER_ADDING_PTP && LWIP_PTP)
uint8_t ptp_tx_tstamp_parse_packet(uint8_t *buffer, struct pbuf *p)
{
//	static int zero_buff_c = 0;
	/* parse 'buffer' to know basically use 'TCR_TXREQ' and append with
	*  'TCR_TSEN_CAP' and/or 'TCR_TS1STEP_EMIT'
	*/
	uint8_t tcr_wr = TCR_TXREQ;
	if (is_issue_ptp_sync_one_step_emit(buffer)) { /* is_sync, one-step */
		//ptp_tx_tstamp_zero(buffer);
		tcr_wr |= TCR_TS1STEP_EMIT;
	
	#if 1	// only master twice
	#if 1 //[check ping]
		/* master's sync */
		do {
			static int n_memo_debug_sync_tsen_cap = 0;
			if (n_memo_debug_sync_tsen_cap < 3) {
				n_memo_debug_sync_tsen_cap++;

			//if (n_memo_debug_sync_tsen_cap < 3) {
			//	tcr_wr |= TCR_TSEN_CAP;
			//	printf(" issue sync 1-step/ %d.memo_debug_sync_tsen_cap, tcr_wr %02x\r\n", n_memo_debug_sync_tsen_cap, tcr_wr);
			//}
			dm_eth_output_data_dump((n_memo_debug_sync_tsen_cap < 3) ? "issueSync test":
				"issueSync last-one", buffer, p->tot_len, (size_t) 14); //dump_data(buffer, (uint16_t) 14);
			}
		} while(0);
	#endif
	#endif
	}
	if (is_issue_ptp_tstamp_tsen(buffer)) {
		tcr_wr |= TCR_TSEN_CAP;
#if 1
		if (is_issue_ptp_tstamp_delayreq(buffer)) { /* one-step */
			//if (slave_count <= DLYREQ_1STEP_COUNT) {
			//}
			tcr_wr |= TCR_TS1STEP_EMIT;
			printf("%s [issue dlyreq, slave optional provide 1-step ts] tcr_wr %02x\r\n", dm_eth_app_mcu_name(NULL), tcr_wr); //PTPD_HEADER_MCU_DESC
		}
#endif
	}

	// Fix sync packet with zero ts before Tx_Req.
	if (tcr_wr & TCR_TS1STEP_EMIT) {
		ptp_tx_tstamp_zero(buffer);
//		printf("%s [ptp tx_tstamp_zero(buffer)].%d\r\n", dm_eth_make_app_mcu_name(NULL), ++zero_buff_c);

		struct pbuf *q;
		uint16_t l = 0; //uint16_t len;
		for (q = p; q != NULL; q = q->next)
			l = l + q->len;
//		printf("dump_data len %u\r\n", l);
//		dump_data(buffer, l);
	}
	return tcr_wr;
}

void ptp_tx_tstamp_pass_to(uint8_t *buffer, struct pbuf *p, uint8_t tcr_wr)
{
	struct ptptime_t timestamp;
	//.uint8_t *buffer = get_TransmitBuffer();
	/* Since, is_issue_ptp_tstamp_tsen()
	 */
	if (tcr_wr & TCR_TSEN_CAP)       // bit.7=1, bit.6=1 check if Transmit TimeStamp Capture & PTP transmit one-step sync packet enable
	{
		impl_dm9051_get_tx_timestamp(&timestamp); //V51_JJ_tx_ptptime(&timestamp, "<stepT1.T3>*V51_TX.11");

		// Store timestamp in pbuf
		p->time_sec = timestamp.tv_sec;
		p->time_nsec = timestamp.tv_nsec;
#if 1
		/* maintain
		 */
		if (is_issue_ptp_tstamp_delayreq(buffer)) { /* one-step */
			printf("%s (slave on ISSUE_DELAYREQ) pk_ts(%u.%09u)\r\n",
				dm_eth_app_mcu_name(NULL),
				timestamp.tv_sec,
				timestamp.tv_nsec);
		}
#endif
	}

	if (is_issue_ptp_tstamp_delayreq(buffer)) {
		delayREQ_T3 = timestamp;
		printf("%s (slave save ISSUE_DELAYREQ, delayREQ_T3) TSEN_CAP ts(%u.%09u)\r\n",
			dm_eth_app_mcu_name(NULL),
			delayREQ_T3.tv_sec,
			delayREQ_T3.tv_nsec);
	}

	/* master's check ('delayRESP_T4' belong to master) */
	if (is_issue_ptp_tstamp_delayresp_packet(buffer)) {
		TimeInternal t4, t3;
		TimeInternal internalTime;
		buffer_ts_time(buffer+14+20+8, &t4);
		internalTime.seconds = t4.seconds; //trapped_delayRESP_T4.tv_sec; //
		internalTime.nanoseconds = t4.nanoseconds; //trapped_delayRESP_T4.tv_nsec; //
		//printf("tx DELAY_RESP: arrived_dlyreq(%u.%09u) master toCHK msgPackDelayResp on send(%u.%09u)\r\n",
		//	trapped_delayRESP_T4.tv_sec, trapped_delayRESP_T4.tv_nsec,
		//	timeTmp.seconds, timeTmp.nanoseconds);
		//printf("tx DELAY_RESP: master msgPackDelayResp on send(%u.%09u)\r\n",
		//	timeTmp.seconds, timeTmp.nanoseconds);

		t3.seconds = master_recv_extra_T3.tv_sec;
		t3.nanoseconds = master_recv_extra_T3.tv_nsec;
		subTime(&internalTime, &internalTime, &t3);

		printf("%s tx DELAY_RESP: master msgPackDelayResp on send(%u.%09u) diff to slave %10d s %11d ns\r\n",
			dm_eth_app_mcu_name(NULL),
			t4.seconds, t4.nanoseconds,
			internalTime.seconds, internalTime.nanoseconds);
		printf("            ..\r\n");
		printf("            ..\r\n");
		printf("            ..\r\n");
		printf("            ..\r\n");
		printf("            ..\r\n");
	}
}
//#endif

/**
 * @brief  Read receive packet header
 *
 * @param  receivedata  Buffer to store header data (4 bytes)
 */
uint16_t cspi_rx_head_ptp(uint8_t *receivedata, uint8_t *ts_bff) //(uint8_t *receivedata)
{
  //uint8_t receivedata[4];
  HAL_read_mem(receivedata, 4);
  HAL_write_reg(DM9051_ISR, 0x80);
  return rx_head_takelen_ptp(receivedata, ts_bff);
}

void cspi_rx_tstamp_mem(uint8_t *receivedata, uint8_t *ts_bff)
{
	if (receivedata[1] & RSR_RXTS_EN)
	{
		if (receivedata[1] & RSR_RXTS_LEN)
		{
			HAL_read_mem(ts_bff, 8);
		}
		else
		{
			HAL_read_mem(ts_bff, 4);
		}
		HAL_write_reg(DM9051_ISR, 0x80);
	}
}

uint16_t rx_head_takelen_ptp(uint8_t *receivedata, uint8_t *ts_bff)
{
  uint16_t rx_len;
  uint8_t rx_status;

  /* Read packet header */
  rx_status = receivedata[1];
  rx_len = receivedata[2] + (receivedata[3] << 8);

  /* Validate packet status and length */
  DM9051_RX_BREAK((rx_status & (RSR_ERR_BITS & ~RSR_PTP_BITS)),
                  return env_err_rsthdlr3("_dm9051f rx_status error : 0x%02x\r\n",
                                          rx_status));
  DM9051_RX_BREAK((rx_len > DRVBUF_POOL_BUFSIZE),
                  return env_err_rsthdlr3("_dm9051f rx_len error : %u\r\n",
                                          rx_len));

  cspi_rx_tstamp_mem(receivedata, ts_bff);
  return rx_len;
}

//Save
int32_t put_rateValue; // should i use int64_t?
//uint8_t put_sign;
void v51_putPtpClockRate(int32_t rateValue/*, uint8_t sign*/)
{
	put_rateValue = rateValue;
	//put_sign = sign;
}

/**
 * @brief  Update PTP clock rate with signed addend
 * @param  signed_addend: Signed adjustment value
 * @retval none
 */
void v51_updatePtpClockRate(int64_t signed_addend)
{
  /* Convert 64-bit addend to 32-bit rate value */
  /* This conversion depends on the specific algorithm used */
  int32_t rate_ppb = (int32_t)(signed_addend >> 16); /* Example conversion */

  v51_putPtpClockRate(rate_ppb);
}

//[ymd solution!]
typedef struct {
    int year;
    int month;
    int day;
} ymd_t;

ymd_t data_ydt;

// 將 Unix 時間戳（秒）轉換為年月日（UTC 時間）
ymd_t *convert_seconds_to_ymd(uint32_t seconds) {
    uint32_t days = seconds / 86400; // 總天數
//  uint32_t rem_sec = seconds % 86400;

    // 從 1970-01-01 開始計算
    int year = 1970;
    int month = 1;
    int day = 1;
    // 逐月扣除天數
    int month_days[] = {31,28,31,30,31,30,31,31,30,31,30,31}, i;

    // 逐年扣除天數
    while (1) {
        int is_leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        int days_in_year = is_leap ? 366 : 365;
        if (days >= days_in_year) {
            days -= days_in_year;
            year++;
        } else {
            break;
        }
    }

    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        month_days[1] = 29; // 閏年 2 月 29 天
    }
    for (i = 0; i < 12; i++) {
        if (days >= month_days[i]) {
            days -= month_days[i];
            month++;
        } else {
            day = days + 1; // 剩餘天數轉為當月第幾天
            break;
        }
    }

    data_ydt.year = year; data_ydt.month = month; data_ydt.day = day; //return (ymd_t){year, month, day};
    return &data_ydt;
}

void internaltime_as_ymd_s(char *head, const s32_t seconds, s32_t nanoseconds)
{
	ymd_t *date;
//	struct ptptime_t ts;
//	ts.tv_sec = seconds;
//	ts.tv_nsec = nanoseconds;
	//print_as_ymd(head, &ts);
	date = convert_seconds_to_ymd(seconds);
	printf("%s:%10d s %11d ns (v51) %04d-%02d-%02d\r\n",
		head, seconds, nanoseconds,
		date->year, date->month, date->day);
}

/**
 * @brief  Print timestamp in year-month-day format
 * @param  head: Header string
 * @param  time: Pointer to timestamp structure
 * @retval none
 */
void print_as_ymd(char *head, const struct ptptime_t *time)
{
  if (time == NULL)
    return;

  /* Convert seconds to calendar date (simplified) */
  /* This is a basic implementation - real implementation would use proper date conversion */
  do {
	  uint32_t days    = time->tv_sec / 86400;
	  uint32_t hours   = (time->tv_sec % 86400) / 3600;
	  uint32_t minutes = (time->tv_sec % 3600) / 60;
	  uint32_t seconds = time->tv_sec % 60;

	  printf("%s %ld days, %02ld:%02ld:%02ld.%09ld\r\n", head ? head : "[TIME]", days, hours, minutes, seconds,
			 time->tv_nsec);
  } while(0);
}

/**
 * @brief  Print timestamp in year-month-day format with round parameter
 * @param  rnd: Rounding parameter
 * @param  head: Header string
 * @param  time: Pointer to timestamp structure
 * @retval none
 */
void print_as_ymd_s(int rnd, char *head, const struct ptptime_t *time)
{
  struct ptptime_t rounded_time = *time;

  if (time == NULL)
    return;

  /* Apply rounding if requested */
  if (rnd > 0 && rnd < 9)
  {
    int i;
    uint32_t divisor = 1;
    for (i = 0; i < (9 - rnd); i++)
    {
      divisor *= 10;
    }
    rounded_time.tv_nsec = (rounded_time.tv_nsec / divisor) * divisor;
  }

  print_as_ymd(head, &rounded_time);
}
#endif
