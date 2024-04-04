# StandUp Mate

**StandUp Mate** is a simplified, effective Arduino-based tool designed to combat office syndrome by encouraging users to stand up and take breaks at regular intervals. This project is a standalone version of [**ELEVATE-AI**](https://github.com/RJTPP/ELEVATE-AI), focused on promoting healthier work habits in a straightforward, user-friendly manner, without the need for AI features and environmental analysis.

## Features

- **Presence Detection**: Utilizes an ultrasonic sensor to detect when a user is at their desk.
- **Timed Reminders**: Encourages regular standing breaks by displaying messages on the OLED screen and flashing an LED light.
- **Standalone Operation**: Operates independently without the need for internet connectivity or external data processing.

## Hardware Components

- ESP32-S3 modules
- OLED Display I2C IIC 0.96″ SSD1306 128x64px
- HC-SR04 Ultrasonic Sensor
- Red, Green, Yellow LED (Optional)
- 220Ω Resistors (For LED lights)
- Push Button

## Software Dependencies

- Arduino IDE

### Arduino Libraries

- [`Wire`](https://www.arduino.cc/reference/en/language/functions/communication/wire/) for I2C communications with sensors and display.
- [`Adafruit_SSD1306`](https://github.com/adafruit/Adafruit_SSD1306) for the OLED display control.
- [`HCSR04`](https://github.com/gamegine/HCSR04-ultrasonic-sensor-lib) for the ultrasonic distance measurements.

## Directories
```
📦 StandUp Mate
 ┃
 ┣ 📂 standUpMate
 ┃ ┣ 📜 hardwareConfig.h       # Hardware pin configuration
 ┃ ┣ 📜 standUpMate.h          # Header file for the screen code
 ┃ ┗ 📜 standUpMate.ino        # Main code for the screen ESP32 module
 ┃
 ┣ 📜 LICENSE                  # License file
 ┣ 📜 README.md                # This file
 ┗ 📜 Schematic.pdf
```

## Installation

### Wiring


| Component          | ESP32-S3 Pin |
| ------------------ | ------------ |
| Switch             | 2            |
| Ultrasonic echo    | 4            |
| Ultrasonic trigger | 5            |
| LED (green)        | 40           |
| LED (yellow)       | 41           |
| LED (red)          | 42           |
| OLED SCL pin       | 47           |
| OLED SDA pin       | 48           |

For more detail, please refer to the [`schematic.pdf`](/schematic.pdf) file.

### Software Configuration

1. Install the **Arduino IDE** on your computer.
2. Download and install the required libraries (`Adafruit SSD1306` and `HCSR04`) through the Library Manager in the Arduino IDE.
3. Open the [`standUpMate.ino`](/standUpMate/standUpMate.ino) sketch with the Arduino IDE.
4. Modify the [`hardwareConfig.h`](/standUpMate/hardwareConfig.h) file to configure the ESP32 pin definitions to match your hardware setup.
5. Update the sketch with your preferred timer settings and thresholds for the ultrasonic sensor.
6. Upload the sketch to your Arduino board.

<details>

<summary>See <code>hardwareConfig.h</code></summary>

<br>

```c
// hardwareConfig.h

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define REFRESH_DELAY   16

#define OLED_SCL_PIN    47
#define OLED_SDA_PIN    48

#define US_ECHO_PIN     4
#define US_TRIGGER_PIN  5

#define LED_RED_PIN     42
#define LED_YELLOW_PIN  41
#define LED_GREEN_PIN   40
#define SWITCH_PIN      2
```

</details>

## Usage

1. Power up **StandUp Mate** by connecting it to a USB power source or battery.
2. The device will automatically start monitoring for user presence.
3. Upon detecting a user, the timer begins counting down to the next break reminder.
4. When it's time for a break, the OLED display will show a message, and the LED will start flashing.
5. Stand up and take a break. The device will detect when you've left your seat.
6. Return to reset the timer automatically or use the reset button for manual reset.


## License

This project is licensed under the MIT License - see the [LICENSE](/LICENSE) file for details.

## Developed by

Rajata Thamcharoensatit ([@RJTPP](https://github.com/RJTPP))
