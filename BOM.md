# OpenMantaClaus Bill of Materials (BOM)

**Estimated Total Cost:** `$2,200 – $2,700 AUD` *(including conversion & delivery considerations)*

### 🏷️ Supplier Acronym Legend
* **BR**: Blue Robotics
* **TB**: Taobao
* **AE**: AliExpress
* **AMZ**: Amazon
* **OV**: Ovonic
* **OPT**: Optii
* **KM**: Koenig Machinery
* **GS**: Gold Supplier
* **Local**: Local Supplier

---

## 📋 Categorized Bill of Materials

### 🔹 Enclosure & Structure

| Item | Qty | Price / Cost | Link | Notes |
| :--- | :---: | :--- | :---: | :--- |
| **4inchx300mm acrylic tube + Oring flanges** | 1 | `$330 USD` | [BR](https://bluerobotics.com/store/watertight-enclosures/locking-series/wte-locking-tube-r1-vp/) | Alternative ROVMaker parts can be used to save on costs but the compatibility with the electronics tray is not guaranteed and may require modification. Get endcap with more holes if you want to add more modifications. |
| **4inch aluminum endcap 10xM10 hole** | 1 | `$42 USD` | [BR](https://bluerobotics.com/store/watertight-enclosures/locking-series/wte-end-cap-vp/) | same as above |
| **4inch 500m PC dome** | 1 | `$42 USD` | [BR](https://bluerobotics.com/store/watertight-enclosures/locking-series/wte-dome-vp/) | same as above |
| **Penetrators** | 8 | `$91 USD + 33 CNY` | [BR](https://bluerobotics.com/store/cables-connectors/penetrators/wet-link-penetrator-vp/) / [TB](https://world.taobao.com) | 5x 5.5LC, 2x6.5LC, 1xROVMaker M10 : This may change if you use different thrusters. OR use the standard ROVMaker M10 penetrators for everything for flexibility (Recommended for cost, with the downside of being permanent). |
| **3mmx550mmx450mm acrylic sheet** | 1 | `~$20 AUD` | [KM](https://koenigmachinery.com.au/products/red-acrylic-new-low-price?variant=53416719646888) | any local supplier/colour of choice, size specified is minimum size |
| **2mmx550mmx450mm stainless steel sheet** | 1 | `~$100 AUD` | — | outsourced machining, or in house with a laser cutter. Price is outsourced machining cost. |

### 🔹 Sensors & Cameras

| Item | Qty | Price / Cost | Link | Notes |
| :--- | :---: | :--- | :---: | :--- |
| **depth sensor** | 1 | `$80 USD` | [BR](https://bluerobotics.com/store/sensors-cameras/sensors/bar-depth-pressure-sensor/) | ROVMaker alternative 30BA can also be used (much cheaper but untested) |
| **OpenMV H7 (or above)** | 1 | `~$100 AUD` | Local | This is not necessary, any monocular camera will do for the bottom camera. I used this because I already had it on hand. One big benefit to this is using VIO by scanning the floor completely offboard from the pi. Enabling this is probably the most valuable extension to the project. |
| **Emeet 4K Camera** | 1 | `$45 AUD` | [AE](https://www.aliexpress.com/item/1005007493158463.html) | — |

### 🔹 Control, Power & Electronics

| Item | Qty | Price / Cost | Link | Notes |
| :--- | :---: | :--- | :---: | :--- |
| **pressure relief valve** | 1 | `$32 USD OR 145 CNY` | [BR](https://bluerobotics.com/store/watertight-enclosures/enclosure-accessories/prv-vp/) / [TB](https://world.taobao.com) | BlueRobotics version was used, but ROVMaker alternatives are equivalent |
| **rotary switch** | 1 | `$28 USD OR 109 CNY` | [BR](https://bluerobotics.com/store/comm-control-power/switches/switch-vp/) / [TB](https://world.taobao.com) | same as above |
| **navigator flight controller** | 1 | `$220 USD` | [BR](https://bluerobotics.com/store/comm-control-power/control/navigator/) | — |
| **Raspberry pi 4b 8gb** | 1 | `~$250 AUD` | Local | any cheap local supplier |
| **brushless ESC 45A** | 2 | `$20 AUD` | [AE](https://www.aliexpress.com/item/1005007500019321.html) | — |
| **servo voltage stepdown** | 1 | `$3 AUD` | [AE](https://www.aliexpress.com/item/1005009769162034.html) | — |
| **pi voltage stepdown** | 1 | `$50 AUD` | [OPT](https://optii.com.au/products/6a-bec-voltage-regulator-6-24v-input-5v-6v-8-4v-12v-output-screw-terminals) | A cheaper aliexpress 5V6A is strongly recommended for this with minimal cad modifications. I just had this one lying around. |
| **100A DC-DC relay** | 1 | `13 CNY` | [TB](https://e.tb.cn/h.RyOAYOxIhQKUoRJ?tk=RGpjgnHRwd8) | — |
| **E-Stop IP68 box** | 1 | `5 CNY` | [TB](https://item.taobao.com/item.htm?id=678862093482) | I got the box with no holes, and drilled holes myself as needed. |
| **E-Stop switch** | 1 | `~$20 AUD` | — | Any E-stop style switch works, can alternatively use a magnetic hall effect switch as well. |
| **Power rails** | 2 | `~15 AUD` | [AE](https://www.aliexpress.com/item/1005004673974312.html) | 6P fis and is recommended. one black, one red. |
| **barrier screw terminals** | 2 | `~$6 AUD` | [AE](https://www.aliexpress.com/item/1005002635711842.html) | X3-3012 30A, can cut this up top separate it |
| **electronics wires/connectors** | 1 | `~$20 AUD` | — | Xt90 connector, M-F jumper cables, RCY connectors for ease of use and assembly. Soldering is necessary, crimping is also used but not required. |

### 🔹 Thrusters & Actuators

| Item | Qty | Price / Cost | Link | Notes |
| :--- | :---: | :--- | :---: | :--- |
| **small thruster** | 3 | `285 CNY` | [TB](https://item.taobao.com/item.htm?id=553255419051&skuId=3396228553275) | 1 CW, 2CCW |
| **big thruster** | 2 | `516 CNY` | [TB](https://item.taobao.com/item.htm?id=672223638935) | 1 CW, 1 CCW |
| **underwater servo** | 1 | `$84 USD` | [GS](https://www.goldsupplier.com/provide/p173038070.html) | or ROVMaker alternative, bit more expensive |

### 🔹 Hardware, Fasteners & Framing

| Item | Qty | Price / Cost | Link | Notes |
| :--- | :---: | :--- | :---: | :--- |
| **fishing magnet** | 1 | `~$4 AUD` | [AE](https://www.aliexpress.com/item/1005008275255309.html) | D25 19kg, can get a stronger one as well |
| **neodymium magnets 15x5mm** | 10 | `~$10 AUD` | [AE](https://www.aliexpress.com/item/1005010487425207.html) | any magnet works, depending on your ball design. |
| **waterproof epoxy putty** | 1 | `$20 AUD` | [AMZ](https://www.amazon.com.au/JB-Weld-JB-8277-Water-Epoxy-Putty/dp/B0886DLQRL) | — |
| **M3 hardware** | 1 | `~$25 AUD` | — | miscellaneous m3 hardware (bolts, 10-25mm), nuts, washers, M-F 10mm standoffs |
| **M4 hardware** | 1 | `~$20 AUD` | — | miscellaneous m4 hardware (bolts, 10-25mm), nuts, washers |
| **PETG/PLA+/PCTG filament 1kg roll** | 2 | `$35 AUD` | — | any supplier/colour of choice |

### 🔹 Batteries

| Item | Qty | Price / Cost | Link | Notes |
| :--- | :---: | :--- | :---: | :--- |
| **4s lipo batteries** | 2 | `$100 AUD` | [OV](https://www.ovonicshop.com/checkouts/cn/hWN4wA9AchssAm3JbhMg0oo0/en-au) | any equivalent spec/size battery would do, brand doesn't matter |

---

## 💡 Notes & Recommendations

* **Wiring Assemblies:** Some experience with and access to crimping tools and standard connectors (like Dupont and RCY connectors) is strongly recommended for reliable wiring.
* **Specialized Enclosure Tools:** Apart from the listed parts, some extra tools are recommended to ensure watertightness and safety:
  * **[Vacuum Pump Kit](https://bluerobotics.com/store/watertight-enclosures/enclosure-tools-supplies/vacuum-pump-kit-r2-rp/)**: Highly recommended for pressure testing the enclosures before water deployment.
  * **[WLP Bulkhead Wrench](https://bluerobotics.com/store/cables-connectors/tools/wlp-bulkhead-wrench/)**: Recommended for securely tightening the cable penetrators.
  * *Note: Alternative approaches/tools are possible as well.*
* **Penetrators:** Standard ROVMaker M10 penetrators offer the best cost efficiency, though they provide a permanent seal.
* **Flight Controller / Navigation:** The OpenMV camera setup supports visual-inertial odometry (VIO) for bottom tracking offloaded from the Raspberry Pi, serving as a highly recommended system extension.
* **Enclosure:** Alternative ROVMaker parts can reduce costs for the main enclosure tube and endcaps, but may require custom modifications for fitment with the internal tray.
