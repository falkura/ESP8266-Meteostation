
# MeteX

MeteX is home meteostation based on NodeMCU v2.0 with sensors BME680 and MQ135, info display and control button.
<details><summary>MeteX Circuit Preview</summary>
<p>

![Metex Preview](https://github.com/falkura/ESP8266-Meteo/assets/52701619/457310f8-2d26-41ab-b816-22f6bd7d36a4)
</p>
</details>


## 🔨 Installation

1. Install [VSCode](https://code.visualstudio.com/) as main IDE.

2. Install [PlatformIO](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide/) for embedded software development.

3. Install driver [CP210x](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads) USB to UART Bridge Virtual COM Port (VCP) for NodeMCU operations.

4. Install [Python](https://www.python.org/) **3.5** or higher.

> [!IMPORTANT]
> Make sure you check the option Add Python to PATH!

5. Clone this project.



## 🚪 Open Project

Go to **VSCode**, navigate to **PlatformIO** tab, **open folder** and choose cloned project folder.

> [!WARNING]
> You shouldn’t delete, modify or move the folders and the **platformio.ini** file. Otherwise, you will no longer be able to compile your project using PlatformIO.



## 🧱 Building

1. To compile C++ into compatible .elf file navigate to **PlatformIO tab**, then click **General** -> **Build**.

2. To compile project files from **data** folder to compatible .bin file navigate to **PlatformIO tab**, then click **Platform** -> **Build Filesystem Image**.



## 🚀 Uploading

1. To upload code - **PlatformIO** -> **General** -> **Upload**.

2. To upload fs image - **PlatformIO** -> **Platform** -> **Upload Filesystem image**.

> [!IMPORTANT]
> REMOVE VIN AND GND BEFORE UPLOAD!



## 🌡️ [BME680](https://www.bosch-sensortec.com/products/environmental-sensors/gas-sensors/bme680/)

BME680 sensor measuring relative:

* Ambient temperature. `-40 to 85 ± 1.0ºC`
* Humidity. `0 to 100 ± 3%`
* Barometric pressure. `300 to 1100 ± 1 hPa`
* Gas: Volatile Organic Compounds (VOC) like ethanol and carbon monoxide.


### Sea level pressure ([adafruit reference](https://adafruit.github.io/Adafruit_BME680/html/class_adafruit___b_m_e680.html#a3caae92aa981508f0084b11b1fed4883))

```bash
#define SEALEVELPRESSURE_HPA (1013)
```

This variable saves the pressure at the sea level in hectopascal (is equivalent to milibar). This variable is used to estimate the altitude for a given pressure by comparing it with the sea level pressure.

You can find sea level pressure of your current location at [Ventusky](https://www.ventusky.com/) or similar services.

If you have several airports near your location, you can get more accurate SLP. You can do it according to [this guide](https://www.youtube.com/watch?v=Wq-Kb7D8eQ4).

### Oversampling ([adafruit reference](https://adafruit.github.io/Adafruit_BME680/html/class_adafruit___b_m_e680.html#a640ee0a0cb7ca57af30e8408260cc6e6))

To increase the effective resolution of measurements and reduce noise, the BME680 supports oversampling (taking multiple samples and averaging them together).

* `setTemperatureOversampling()`
* `setHumidityOversampling()`
* `setPressureOversampling()`

These methods accepts oversampling level up to 16 X.


### IIR filter ([adafruit reference](https://adafruit.github.io/Adafruit_BME680/html/class_adafruit___b_m_e680.html#a42f25a4f258aad9abad4abb6bd95ec77))

The BME680 sensor integrates an internal IIR filter to reduce short-term changes in sensor output values caused by external disturbances.

* `setIIRFilterSize()`


### Gas sensor ([adafruit reference](https://adafruit.github.io/Adafruit_BME680/html/class_adafruit___b_m_e680.html#a2e6a61b5441c51bf5e44c3af3ee3fec8))

The gas sensor integrates a heater. Set the heater profile using the `setGasHeater(TEMPERATURE, DURATION)` method. 



## 💨 MQ135

The MQ135 is an air quality sensor module that designed to detect the content and quantity of harmful and hazardous gases in the air such as NH3, NOx, alcohol vapour, petrol, smoke, CO2, etc.


### References

- [**Manual**](https://www.winsen-sensor.com/d/files/PDF/Semiconductor%20Gas%20Sensor/MQ135%20(Ver1.4)%20-%20Manual.pdf)
- [**Datasheet**](https://www.olimex.com/Products/Components/Sensors/Gas/SNS-MQ135/resources/SNS-MQ135.pdf)



## ⚖️ Calibration

To get an accurate readings from BME680 and from MQ135 it is important to calibrate sensors. 

> [!NOTE]
> Temperature, humidity and MQ135 should only be calibrated once, whereas pressure calibration should be done every time the meteostation is moved to a new location (only relevant for mobile meteostations).


### CO2

To calibrate MQ135 - put your meteostation outside where there is enough fresh air (ideally at 20°C and 33% humidity according to the datasheet) and leave it powered on for 12-24 hours to burn it in and stabilize.

Then you can restart meteostation via pressing **RST** button on your NodeMCU.


### Temperature and Humidity

To calibrate temperature and humidity - leave a good thermohydrometer (thermometer with humidity sensor) near the meteostation and adjust offset values in code:

- `temperature_offset` for temperature difference
- `humidity_offset` for humidity difference

> [!NOTE]
> The temperature and humidity sensors are good enough, so calibrating them is not that necessary.

### Pressure

To find out how to calibrate pressure module read [this section](https://github.com/falkura/ESP8266-Meteo/tree/master?tab=readme-ov-file#sea-level-pressure-adafruit-reference).


### VOC (BME680 gas sensor)

To calibrate the gas sensor, you need to find an environment with **good air quality**, leave meteostation there for a few minutes, and then repeat this in an environment with **poor air quality**.



## 🖥️ OLED display

**SSD1306-128x64** - Is a simple screen I used to display sensor information directly on the meteostation.

More info about the screen you can find here - [**Datasheet**](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)


### Control 
To change the information displayed, you need to press the button and hold it down until the next screen appears. Then, you need to release the button, and repeat this process for another screen.



## Wi-Fi
    * ssid
    * password

## Troubleshooting
    * reset+flash
    * additional driver
    * nodemcu flasher

## License
    * mit