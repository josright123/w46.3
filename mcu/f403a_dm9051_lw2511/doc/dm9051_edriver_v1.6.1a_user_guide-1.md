# **dm9051 edriver v1.6.1a user guide**

此驅動目錄（dm9051_edriver_v1.6.1a）為 Davicom DM9051 乙太網路控制器的 MCU 驅動，支援 uIP、LWIP 等 TCP/IP 協議棧，並可在多種 MCU（如 STM32、AT32）上運作。以下說明其主要接口與功能 callstack：

##### *1. **主要接口（API）***

###### 驅動核心 API（core/dm9051.h、core/dm9051.c）
###### - 初始化與設定

int dm9051_conf(void);
- 初始化 SPI 與中斷（根據設定），設置 HAL 層。

const uint8_t *dm9051_init(const uint8_t *adr);
- 驗證晶片、設置 MAC、初始化收發參數與中斷。

###### - 收發封包

uint16_t dm9051_rx(uint8_t *buf);
- 檢查是否有封包，讀取封包資料到 buffer。

void dm9051_tx(uint8_t *buf, uint16_t len);
- 發送封包，支援標準與連續傳送模式。

###### - 中斷

void dm9051_interrupt_set(uint32_t exint_line);
int dm9051_interrupt_get(void);
void dm9051_interrupt_reset(void);
- 中斷事件設置、查詢與重置。
###### - 狀態

uint8_t *identified_eth_mac(void);  
- 取得以設定的mac地址。
uint16_t dm9051_status(void);
- 取得目前控制器狀態（如連線狀態）。
void dm9051_read_regs_info(uint8_t *stat);
- 取得目前控制器狀態（如連線狀態）。
void dm9051_read_rx_pointers(uint16_t *rwpa_wt, uint16_t *mdra_rd);
- 取得目前RX指標。

###### *HAL 硬體抽象層接口（hal/ 目錄）*

###### - SPI 操作
- void hal_spi_initialize(void);
- uint8_t hal_read_reg(uint8_t reg);
- void hal_write_reg(uint8_t reg, uint8_t val);
- void hal_read_mem(uint8_t *buf, uint16_t len);
- void hal_write_mem(uint8_t *buf, uint16_t len);
###### - 中斷操作
- uint32_t hal_int_initialize(void);
- void hal_enable_mcu_irq(void);
- void hal_disable_mcu_irq(void);
- uint32_t hal_irqline(void);

##### ***2. 應用流程（Callstack）**

###### (A) 初始化階段

1. 硬體初始化
- dm9051_conf() → 呼叫 hal_spi_initialize()、hal_int_initialize()（若為中斷模式）

2. 核心初始化
- dm9051_init(mac_addr) → 驗證晶片、設置 MAC、呼叫 init_setup() 完成底層初始化
###### (B) 封包收發

###### - 接收
- dm9051_rx(buf) → cspi_rx_ready() → cspi_rx_read(buf, cspi_rx_head())
###### - 發送
- dm9051_tx(buf, len) → dm9051_single_tx(buf, len)（或連續模式）

###### (C) 中斷處理（如啟用中斷模式）

- MCU 中斷服務程式（如 EXINT9_5_IRQHandler）呼叫 dm9051_interrupt_set()
- 主循環或任務查詢 dm9051_interrupt_get()，處理後呼叫 dm9051_interrupt_reset()

##### 3. 典型應用程式碼（參考 user guide）

```c
void vuIP_Conf(void) {
    dm9051_conf();
}

void vuIP_Init(void) {
    const uint8_t *raddr = dm9051_init(domain_addr);
    if (raddr)
        memcpy(&uip_ethaddr.addr[0], raddr, MAC_ADDR_LENGTH);
}

int vuIP_Process(void) {
    uip_len = dm9051_rx(uip_buf);
    ...
    if (uip_len > 0) {
        uip_arp_out();
        dm9051_tx(uip_buf, uip_len);
    }
    ...
}
```

##### 4. HAL 層移植說明

- 需根據 MCU 實作 SPI 與中斷相關的 hal_xxx 函數（見 hal/ 目錄範例）。
- 只要實作好 HAL 層，核心驅動不需修改即可移植到不同 MCU。

##### 5. 支援模式

- Polling mode：主循環輪詢收發。
- Interrupt mode：中斷觸發收發，需 MCU 支援外部中斷。

##### 總結

- 核心 API：dm9051_conf、dm9051_init、dm9051_rx、dm9051_tx、dm9051_status、中斷相關。
- HAL API：SPI 與中斷相關的硬體操作。
- 應用流程：初始化 → 收發封包（輪詢或中斷）→ 狀態查詢。
- 移植性：只需實作 HAL 層即可支援不同 MCU。
