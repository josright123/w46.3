# DM9051 Driver Interrupt Mode Configuration Guide

This guide explains how to configure the operating mode of the DM9051 network driver, specifically switching between "Interrupt Mode" and "Polling Mode".

## Overview of Operating Modes

The driver can operate in two main modes for handling network events:

### 1. Interrupt Mode (`MODE_INTERRUPT`)

In this mode, the DM9051 hardware device sends a signal (an interrupt) to the CPU when an event occurs (e.g., a new network packet has been received). This is a highly efficient, event-driven method and is the **recommended configuration for standard operation**. It allows the system to perform other tasks until the hardware explicitly needs attention.

### 2. Polling Mode (`MODE_POLL`)

In this mode, the driver does not wait for a signal from the hardware. Instead, the CPU periodically and repeatedly checks the device to see if there are any new events. This method is less efficient than interrupt mode as it consumes more CPU cycles, but it can be a useful alternative for debugging purposes or in environments where interrupts are problematic.

**The default mode for this driver version is `MODE_POLL`.**

## How to Change the Operating Mode

To change the operating mode, you need to edit the source file `dm9051.c` and recompile the driver.

**File to Edit:** `dm9051.c`

**Steps:**

1.  Open the file `dm9051.c` in a text editor.
2.  Locate the `confdata` structure definition. It will look similar to this:

    ```c
    /* Default driver configuration */
    const struct driver_config confdata = {
        .release_version = "lnx_dm9051_kt6631_r2502_v3.9.1",
        .interrupt = MODE_POLL, /* MODE_INTERRUPT or MODE_INTERRUPT_CLKOUT */
        .mid = MODE_B,
        ...
    };
    ```

3.  Modify the `.interrupt` line to set your desired mode:

    *   **For Interrupt Mode (Recommended):**
        Change the line to:
        ```c
        .interrupt = MODE_INTERRUPT,
        ```

    *   **For Polling Mode:**
        Ensure the line is set to:
        ```c
        .interrupt = MODE_POLL,
        ```

4.  Save the `dm9051.c` file.

5.  Recompile the driver using the provided `Makefile` for the changes to take effect.

---
*Note: There is also a `MODE_INTERRUPT_CLKOUT` option available, which is a variation of the interrupt mode. Please refer to the hardware documentation for details on its specific use case.*
