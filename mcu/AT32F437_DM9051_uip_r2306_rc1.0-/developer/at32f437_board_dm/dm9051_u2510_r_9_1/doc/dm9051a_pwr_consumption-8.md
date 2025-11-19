# **DM9051A Power Consumption Test Report**


**功耗測試環境：**  
此測試为芯片DM9051A以太网控制器芯片的功耗量測，使用雅特力AT32F437 mcu板加上Davicom DM9051A demo board (32QFN DM9051A Demo Board V1.0 )進行測試.

**測試各階段說明：**  
系統操作流程,開機後先進入phy power-down模式進行量測,然後轉換到phy power-on模式進行量測,此phy power-up階段phy仍為link-down狀態為,經上述power-down/power-up反覆3次後,系統進入正常開機的網路功能應用模式,經DM9051A以太网控制器芯片Link-up連線並進行收送數據包狀態下量測全速運作的最大功耗數據. 操作中相應的日誌如下:
- [CHK] at32f437_spi1_dma spi DMA Running...
- [CLK] 250000000(sclk_freq) 
- SPI CLK set to 15Mhz dma
- [DRIVER POL mode] DM9051 found: 9051
- [DRIVER POL mode] DM9051 found: 9051, set phy down (1f: 01)
- [DRIVER POL mode] DM9051 found: 9051, set phy up (1f: 00)
- mac address: reg 00 60 6e e2 87 7b
- [DRIVER POL mode] startup link up - 100M Full duplex

### Phy power-on

| Power-on | 項目     | 數值          | 說明                       |
| :------: | ------ | ----------- | ------------------------ |
|          | **電壓** | 3.3 V       | MCU週邊設備的標準電壓             |
| Link up  | **電流** | **53.8 mA** | PHY上電且Link up傳輸就緒通訊進行中電流 |
|          | **功耗** | ~177.54 mW  | V×I計算值，連線運作時之典型功耗        |
(+ 0.0 mA), (100%),  Linkup at 100M Full duplex


| Power-on  | 項目     | 數值          | 說明                      |
| :-------: | ------ | ----------- | ----------------------- |
|           | **電壓** | 3.3 V       | MCU週邊設備的標準電壓            |
| Link down | **電流** | **23.1 mA** | PHY上電但未連線（Link down）之電流 |
|           | **功耗** | ~76.23 mW   | 較Link up約降57%的待機功耗      |
(- 30.7 mA), (-57%)

### Phy power-down

|  Power-down  | 項目     | 數值         | 說明                    |
| :----------: | ------ | ---------- | --------------------- |
|              | **電壓** | 3.3 V      | MCU週邊設備的標準電壓          |
| Phy inactive | **電流** | **5.3 mA** | **非常低**，屬於待機/睡眠模式的典型值 |
|              | **功耗** | ~17.49 mW  | 較Link up約降90%功耗極低     |
(-48.5 mA), (-90%)


**建議：**  
要全面評估，您還需要查看該網卡在其他系統的不同工作場景下的功耗。

**市面消息：**  
前一版芯片典型DM9051以太网控制器芯片功耗，实际耗电会受具体使用环境和负载影响.
- 工作电压3.3V
- 耗电电流约130~170mA
- 功耗约429mW到561mW

**觀察**
- Link up時**53.8 mA**、Phy power-down時**5.3 mA**，差異約90%(48.5 mA) 。
	- link up ---> Phy power-down 降低達90%大量負載電流
- 在Phy power-down時電流**5.3 mA**(約17.49 mW) 顯示PHY關閉電源系統有明顯降下負載。
	- 在Phy power-down的時候負載功耗極低僅5.3 mA
- 數據(在17.49~177.54 mW) 顯著低於市面前一版典型DM9051範圍(約429–561 mW) ，符合預期。
