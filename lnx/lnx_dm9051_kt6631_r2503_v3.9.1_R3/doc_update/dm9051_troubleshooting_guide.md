# DM9051 SPI Fast Ethernet Driver - Trobbleshooting Guide

## Table of Contents

1. [Troubleshooting](#troubleshooting)
2. [Examples](#examples)

---

## Overview

The DM9051 driver is a Linux network driver for the Davicom DM9051 SPI Fast Ethernet controller. This guide focuses on configuring and tuning the driver for optimal network performance on different hardware platforms.

## Troubleshooting

### Low Throughput Issues

**Symptoms**: Lower than expected bandwidth or packet rate

**Diagnosis Steps**:
1. Check current configuration:
   ```bash
   dmesg | grep -i dm9051
   ```

2. Verify interrupt mode is enabled:
   - Look for "Operation: Interrupt mode" in dmesg
   - Not "Operation: Polling mode"

3. Check SPI transfer mode:
   - Look for "SPI_XFER_MEM= burst mode" (preferred)
   - Not "SPI_XFER_MEM= alignment mode"

**Solutions**:
- Set to enable `BURST_MODE_FULL` if using alignment mode
- Set to enable interrupt mode if using polling
- Set to enable checksum offload
- Check SPI bus speed which is configured in the device tree

### High CPU Usage

**Symptoms**: CPU usage > 50% for network processing

**Diagnosis**:
1. Check if polling mode is active
2. Check if checksum offload is disabled
3. Monitor packet rate vs. CPU usage

**Solutions**:
- Switch to interrupt mode (`MODE_INTERRUPT`)
- Enable checksum offload (`DEFAULT_CHECKSUM_ON`)
- Increase polling delays if polling mode is required
- Check for excessive error handling (monitor error counters)

### Packet Loss or Errors

**Symptoms**: RX/TX errors, dropped packets

**Diagnosis**:
1. Check error counters:
   ```bash
   ethtool -S eth0 | grep error
   ```

2. Enable monitoring:
   ```c
   .force_monitor_rxb = FORCE_MONITOR_RXB,
   .force_monitor_tx_timeout = FORCE_MONITOR_TX_TIMEOUT,
   ```

**Solutions**:
- Increase TX timeout if timeouts occur
- Check SPI bus stability (reduce speed if needed)
- Switch to alignment mode if burst mode causes errors
- Adjust block sizes for alignment mode

### SPI Transfer Errors

**Symptoms**: SPI transaction failures, regmap errors

**Solutions**:
- Reduce SPI bus speed in device tree
- Switch from burst mode to alignment mode
- Reduce block sizes in alignment mode
- Verify SPI controller compatibility

### Configuration Not Applied

**Symptoms**: Changes don't take effect

**Solution**:
1. Rebuild the driver module
2. Unload and reload the module:
   ```bash
   rmmod dm9051
   insmod dm9051
   ```
3. Check dmesg for configuration messages
4. Verify no compilation errors

---

## Examples

### Example 1: High-Performance Configuration for RPi4

```c
const struct driver_config confdata = {
    .release_version = "lnx_dm9051_kt6631_r2503_v3.9.1_R2",
    .interrupt = MODE_INTERRUPT,  // Use interrupts
    .mid = MODE_B,  // RPi4 optimized
    .mod = {
        [MODE_A] = { /* ... */ },
        [MODE_B] = {
            .test_info = "RPi4 High Performance",
            .skb_wb_mode = SKB_WB_ON,
            .tx_mode = FORCE_TX_CONTI_ON,  // Enable TX continuation
            .checksuming = DEFAULT_CHECKSUM_ON,  // Enable checksum offload
            .align = {
                .burst_mode = BURST_MODE_FULL,  // Full burst mode
                .tx_blk = 0,
                .rx_blk = 0,
            }
        },
        [MODE_C] = { /* ... */ }
    }
};
```

### Example 2: Custom Alignment Mode Configuration

```c
[MODE_A] = {
    .test_info = "Custom Alignment Mode",
    .skb_wb_mode = SKB_WB_ON,
    .tx_mode = FORCE_TX_CONTI_OFF,
    .checksuming = DEFAULT_CHECKSUM_ON,
    .align = {
        .burst_mode = BURST_MODE_ALIGN,  // Use alignment mode
        .tx_blk = 64,  // 64-byte TX blocks
        .rx_blk = 128,  // 128-byte RX blocks
    }
}
```

### Example 3: Polling Mode with Custom Delays

```c
// In dm9051.c
const struct driver_config confdata = {
    .interrupt = MODE_POLL,  // Use polling mode
    // ...
};

// In dm9051.h
const struct eng_config engdata = {
    .force_monitor_rxb = FORCE_SILENCE_RXB,
    .force_monitor_rxc = FORCE_SILENCE_RX_COUNT,
    .force_monitor_tx_timeout = FORCE_SILENCE_TX_TIMEOUT,
    .sched = {
        .delayF = {0, 1, 2, 1, 1},  // Custom polling delays
        .nTargetMaxNum = 5,  // Use all 5 delay values
    },
    .tx_timeout_us = 3000,  // 3 ms timeout
};
```

### Example 4: Debug Configuration

```c
[MODE_B] = {
    .test_info = "Debug Mode",
    .skb_wb_mode = SKB_WB_ON,
    .tx_mode = FORCE_TX_CONTI_OFF,  // Standard mode for debugging
    .checksuming = DEFAULT_CHECKSUM_OFF,  // Software checksum for debugging
    .align = {
        .burst_mode = BURST_MODE_ALIGN,  // More compatible
        .tx_blk = 32,
        .rx_blk = 64,
    }
}

// In dm9051.h
const struct eng_config engdata = {
    .force_monitor_rxb = FORCE_MONITOR_RXB,  // Enable monitoring
    .force_monitor_rxc = FORCE_MONITOR_RX_COUNT,
    .force_monitor_tx_timeout = FORCE_MONITOR_TX_TIMEOUT,  // Enable timeout monitoring
    // ...
};
```

---

**Document Version**: 1.0  
**Last Updated**: 2025-11-20  
**Driver Version**: r2503_v3.9.1_R2

