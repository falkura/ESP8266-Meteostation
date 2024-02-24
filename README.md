
# MeteX

MeteX is home meteostation based on NodeMCU v2.0 with sensors BME680 and MQ135, info display and control button.


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