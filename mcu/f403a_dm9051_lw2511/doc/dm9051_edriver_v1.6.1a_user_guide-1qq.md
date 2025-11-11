### 補充0. DM9051驅動使用指導文
#### *1. **主要接口（API）***

##### 驅動核心 API（core/dm9051.h、core/dm9051.c）
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
##### *HAL 硬體抽象層接口（hal/ 目錄）*
###### - SPI 操作
- void hal_spi_initialize(void);
- uint8_t hal_read_reg(uint8_t reg);
- void hal_write_reg(uint8_t reg, uint8_t val);
- void hal_read_mem(uint8_t *buf, uint16_t len);
- void hal_write_mem(uint8_t *buf, uint16_t len);
#### ***2. 應用流程（Callstack）**

##### (A) 初始化階段

1. 硬體初始化
- dm9051_conf() → 呼叫 hal_spi_initialize()、hal_int_initialize()（若為中斷模式）

2. 核心初始化
- dm9051_init(mac_addr) → 驗證晶片、設置 MAC、呼叫 init_setup() 完成底層初始化
##### (B) 封包收發

###### - 接收
- dm9051_rx(buf) → cspi_rx_ready() → cspi_rx_read(buf, cspi_rx_head())
###### - 發送
- dm9051_tx(buf, len) → dm9051_single_tx(buf, len)（或連續模式）

#### 3. 典型應用程式碼（參考 user guide）

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

#### 總結

- 核心 API：dm9051_conf、dm9051_init、dm9051_rx、dm9051_tx、dm9051_status、中斷相關。
- HAL API：SPI 與中斷相關的硬體操作。
- HAL層移植：只需實作 HAL 層即可，驅動核心不需修改即可移植支援不同 的MCU。
- 應用流程：初始化 → 收發封包（輪詢或中斷）→ 狀態查詢。
