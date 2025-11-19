# DM9051 Ethernet Driver

This is the driver (dm9051_u2510_r_9_1) for the Davicom DM9051 Ethernet controller.

## Driver Structure

The driver is organized into the following directories:

- `core`: Contains the core logic for the DM9051 driver.
  - `dm9051.h`: Header file for the core driver functions.
  - `dm9051_beta.c`: Source file for the core driver functions.
  - `dm9051_constants.h`: Header file with constants for the DM9051 driver.
- `doc`: Contains documentation for the driver.
  - `dm9051_u2510_r_9_1.md`: User guide for the driver.
  - `support files list.txt`: List of support files.
- `hal`: Contains the Hardware Abstraction Layer (HAL) for different microcontrollers.
  - `hal_at32f403a.h`: HAL header for AT32F403A.
  - `hal_at32f437.h`: HAL header for AT32F437.
  - `hal_stm32f103.h`: HAL header for STM32F103.
  - `hal_stm32f401.h`: HAL header for STM32F401.
  - `INT`: Contains interrupt handling code for different microcontrollers.
  - `SPI1`, `SPI2`: Contains SPI communication code for different microcontrollers and SPI peripherals.

## API

The main API functions are declared in `dm9051.h`:

- [ ] `int dm9051_conf(void)`: Initializes the SPI and interrupt handling.
	- at32f437_spi1 'spi' Running...
	- dm9051_constaants 'INT' Running...
	- SPI CLK 15Mhz
- [ ] `const uint8_t *dm9051_init(const uint8_t *adr)`: Initializes the DM9051 chip.
	- DM9051 found: 9051
- [ ] `uint16_t dm9051_rx(uint8_t *buf)`: Receives a packet from the DM9051.
	- input_packet
- [ ] `void dm9051_tx(uint8_t *buf, uint16_t len)`: Transmits a packet to the DM9051.
	- output_packet
## Usage

The driver can be used in polling mode or interrupt mode.

### Polling Mode

In polling mode, the application periodically calls `dm9051_rx` to check for incoming packets.

### Interrupt Mode

In interrupt mode, the DM9051 generates an interrupt when a packet is received. The application's interrupt handler should then call the necessary functions to process the packet.

For more detailed information on how to use the driver, please refer to the user guide in the `doc` directory.
