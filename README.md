# HaalariLED - WiFi Configurable LED Matrix Display

HaalariLED is an ESP32-based microcontroller project designed to drive LED matrix displays. With WiFi connectivity, you can easily configure various display settings using a web interface. This README.md provides comprehensive instructions on how to use, install libraries, and flash the code onto your ESP32 microcontroller.

## Features

- Configure display text, letter gap, scrolling update delay, text color, background color, WiFi SSID, and password.
- Web-based user interface served on port 80 for easy configuration.
- Factory reset option available by shorting pin 5 to ground.
- Default configuration for an 8x32 display, with the ability to chain multiple displays.
- Supports capital Latin letters and most symbols; automatically converts lowercase letters to uppercase.
- Unsupported characters are displayed as '?'.

## Getting Started

### Prerequisites

Make sure you have the following installed:

- Arduino IDE ([Download](https://www.arduino.cc/en/software))
- ESP32 board support for Arduino IDE ([Instructions](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/))

### Installing Libraries

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

3. Configure the settings in the `config.h` file according to your LED matrix specifications. Pay attention to the LED data pin, display size, and other parameters.

4. Connect your ESP32 to your computer using a USB cable.

5. Select your ESP32 board from **Tools > Board** menu.

6. Select the appropriate COM port from **Tools > Port** menu.

7. Click the "Upload" button in the Arduino IDE to flash the code onto your ESP32.

8. Once the code is uploaded, open the Serial Monitor (**Tools > Serial Monitor**) to view the ESP32's IP address.

9. Connect to the displayed IP address via a web browser to access the configuration interface (e.g., `http://192.168.1.1` or `http://esp.local`).

## Usage

1. Connect to the WiFi network provided by the HaalariLED device.
2. Open a web browser and navigate to the provided IP address.
3. Configure the display settings according to your preferences.
4. Click "Submit" to apply the changes.

## Factory Reset

In case of forgotten wifi credentials or other problems you can perform a factory reset by shorting pin 5 to ground while the device is powered.

## Contributing

Feel free to contribute to the development of HaalariLED by opening issues or submitting pull requests.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

**Note:** Make sure to refer to the official documentation of the libraries and ESP32 board support for any additional information or troubleshooting.