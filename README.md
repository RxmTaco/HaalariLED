![qr_code](https://github.com/RxmTaco/HaalariLED/assets/117571946/2da6a087-f680-46f5-b719-3a9d9a8a9a44)


# HaalariLED - WiFi Configurable LED Matrix Display

HaalariLED is an ESP32-based microcontroller project designed to drive LED matrix displays. With WiFi connectivity, you can easily configure various display settings using a web interface. This README.md provides comprehensive instructions on how to use, install libraries, and flash the code onto your ESP32 microcontroller.

## Features

- Configure display text, letter gap, scrolling update delay, text color, background color, WiFi SSID, and password.
- Web-based user interface served on port 80 for easy configuration.
- Factory reset option available by shorting pin 5 to ground.
- Default configuration for an 8x32 display, with the ability to chain multiple displays.
- Supports capital Latin letters and most symbols; automatically converts lowercase letters to uppercase.
- Unsupported characters are displayed as '?'.
- Output current is limited in firmware. Default limit is 1.0A

## Getting Started

## Usage
<img src="https://github.com/RxmTaco/HaalariLED/assets/117571946/441f2d54-f6cc-49b3-b1eb-72899b81939c" width="280">

1. Connect to the WiFi network provided by the HaalariLED device, SSID:`ESP-32-HaalariLED` PASSWORD:`12345678`.
2. Open a web browser and navigate to the provided IP address: `http://esp.local` or `http://192.168.1.1`.
3. Change WiFi credentials
4. Configure the display settings according to your preferences.
5. Click "Submit" to apply the changes.

Make sure you are using a power supply that can handle a minimum of 1A output current!

If you are using a weaker power supply, change the current limit in `spec.h` and flash the firmware.

## Flashing your own firmware
### Prerequisites

Make sure you have the following installed:

- Arduino IDE ([Download](https://www.arduino.cc/en/software))
- ESP32 board support for Arduino IDE ([Instructions](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/))

### Installing Libraries

<img src="https://github.com/RxmTaco/HaalariLED/assets/117571946/40fbc814-67db-4ec3-8170-2ec630f4c15c" width="280"> <img src="https://github.com/RxmTaco/HaalariLED/assets/117571946/f32c5526-7cb5-4479-8e80-588fdcfd3fda" width="280"> <img src="https://github.com/RxmTaco/HaalariLED/assets/117571946/2f47d9c4-056e-4ed2-8055-2db4233ca7b3" width="280">


1. Open Arduino IDE.
2. Go to **Sketch > Include Library > Manage Libraries**.
3. Search for and install the following libraries:

   - `WiFi` by Espressif
   - `Adafruit NeoPixel` by Adafruit
   - `Preferences` by espressif
   - `ESPmDNS` by Espressif

### Flashing the Code

1. Clone this repository to your local machine:

   ```bash
   git clone https://github.com/RxmTaco/HaalariLED.git
   ```

2. Open the `HaalariLED.ino` file in Arduino IDE.

3. Configure the settings in the file according to your LED matrix specifications. Pay attention to the LED data pin, display size, and other parameters.

4. Connect your ESP32 to your computer using a USB cable.

5. Select your ESP32 board from **Tools > Board** menu.

6. Make sure the board settings are correct:

<img src="https://github.com/RxmTaco/HaalariLED/assets/117571946/dbbe7d62-54d2-4bba-86d2-e63d61b7923f" width="400">


7. Select the appropriate COM port from **Tools > Port** menu.

8. Click the "Upload" button in the Arduino IDE to flash the code onto your ESP32.

9. Connect to the WebUI IP address via a web browser to access the configuration interface (e.g., `http://192.168.1.1` or `http://esp.local`).

## Issues

**The board is disconnecting while plugged to the PC, it might be due to the PC's USB output current limit.**

Solution:
- Connect to a more powerful supply and change brightness to 0, **OR**
- Start upload and connect board to PC after starting upload, 2 second wait period should allow for new firmware flash

**"Wrong password" while connecting to the ESP's access point**

Solution:
- Factory reset board in case of forgotten password
- Enable `Tools -> Erase All Flash Before Sketch Upload` in Arduino IDE and flash new firmware.

## Factory Reset

In case of forgotten wifi credentials or other problems you can perform a factory reset by pushing the button through the hole in the case while the device is powered.

Or by connecting together RST_PIN and RST_SRC while the device is powered.

By default RST_PIN is 7 and RST_SRC is 8

## Contributing

Feel free to contribute to the development of HaalariLED by opening issues or submitting pull requests.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

**Note:** Make sure to refer to the official documentation of the libraries and ESP32 board support for any additional information or troubleshooting.
