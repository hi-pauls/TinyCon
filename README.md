# TinyCon - A simple Bluetooth, USB and I2C controller implementation.

This is a simple controller implementation meant to serve as a basis for a home-grown game console project
as well as for a Z13 controller attachment project (for now). It was quickly thrown together on a weekend
to deal with the particulars of having multiple DRV2605L controllers as well as the particularities of the
Adafruit Seesaw devices, that take an insane amount of time to update their data and that tend to lock up
for aparently no reason sometimes. Feel free to use the project, but don't expect too much (yet), it doesn't
even use proper C++ as I'd expect from my more long-term projects. Improvements are welcome and may be merged.

## Hardware

The TinyCon is a simple controller that can be used to control a variety of devices. It is based on the nRF52840,
but due to its base on Arduino and Adafruit Feather, should be easy to port to other development boards as well.
Development was done on the following hardware:

- Adafruit Feather nRF52840 Express
- Adafruit Joy FeatherWing (up to 2 at the moment, with room to grow)
- Pimoroni HapticBuzz (DRV2605L) (up to 2 at the moment, with room to grow)
- Pimoroni 9-DoF IMU (ICM20948) (up to 2 at the moment, with room to grow)
- Optional SSD1306 OLED display (for debugging purposes mostly)

Because we want to support one internal and one external instance of each of the I2C devices and the DRV2605L has a
single, fixed I2C address, the internal DRV2605L is connected to a software I2C bus. All other devices are connected
to the hardware I2C0. The nRF52840 does have a second I2C1 bus, that is not set up by default, but configured in the
sketch to be used as the slave port to connect the controller to other builds. Therefore, to not have to connect to
all I2C devices using software I2C, an MCU with at least two hardware I2C controllers is strongly recommended, but
not strictly required.

Below is the connection setup for the master controller consisting of a stack of Joy FeatherWing and MCU, with the
ICM20948 and HapticBuzz wedged in or connected at the bottom using a ProtoWing. The slave is a stack of just a Joy
FeatherWing and a ProtoWing. In both cases, the Stemma connector from the ICM20948 are used to expose the I2C bus
to the outside and the pin connector is soldered to the feather header.

The design makes a point to keep A0 to A5 as well as D0, SCLK, MOSI, MISO, D6, D9, D10 and D13 on the Feather header
free for up to 6 additional analog axis and 8 additional buttons directly connected to the design, these are currently
not supported in code though and would need to be added to InputController.cpp.

                              Reset o
                               3.3V o ---------------------\
               /------ VUSBDIV|AREF o                      |
    /---[100k]-+-[100k]-------- GND o -------------------\ |
    |                  FUT_AXIS1|A0 o     o VBAT         | |
    |                  FUT_AXIS2|A1 o     o EN           | |
    |                  FUT_AXIS3|A2 o  /- o VUSB         | \-- StemmaQt_Slave_3.3V o
    |                  FUT_AXIS4|A3 o  |  o 13|FUT_B5    \---- StemmaQt_Slave_GND  o
    |   o USER_SW      FUT_AXIS5|A4 o  |  o 12|I2C1_SDA ------ StemmaQt_Slave_SDA  o
    |   o LED_BLUE     FUT_AXIS6|A5 o  |  o 11|I2C1_SCL ------ StemmaQt_Slave_SCL  o
    |   o LED_RED    FUT_B1|SCLK|2  o  |  o 10|FUT_B6
    |   o NEOPIXEL   FUT_B2|MOSI|3  o  |  o  9|FUT_B7
    |   o VBATDIV|A6 FUT_B3|MISO|4  o  |  o  6|FUT_B8
    |                         B4|0  o  |  o  5|SOFT_I2C_SDA|DRV2605L-1
    |    DRV2605L-1|SOFT_I2C_SCL|1  o  |  o  7|I2C0_SCL|ICM20948-1|Seesaw-1 --- Stemma_Ext_SCL o
    |                               o  |  o  8|I2C0_SDA|ICM20948-1|Seesaw-1 --- Stemma_Ext_SDA o
    \----------------------------------/
                                          o Stemma_Ext_SCL|ICM20948-2|Seesaw-2|DRV2605L-2
                                          o Stemma_Ext_SDA|ICM20948-2|Seesaw-2|DRV2605L-2

## Software

Building the firmware requires either the Arduino IDE or the makefile-based build system. The following
libraries are required: Wire, Adafruit_TinyUSB, any Bluefruit library, Adafruit_seesaw, Adafruit_ICM20X
(including Adafruit_BusIO and Adafruit_Sensor), SoftWire, AsyncDelay and Adafruit_SleepyDog. If using the
Adafruit Feather nRF52840 Express, the following libraries will also be referenced: Adafruit_LittleFS,
InternalFileSystem, Bluefruit52Lib. The Adafruit_DRV2605 library is not required, since a very simple
driver supporting both TwoWire as well as SoftWire is included in the solution directly, but currently
only supporting the exact setup using the Pimoroni HapticBuzz.

Optionally: Adafruit_NeoPixel and/or Adafruit_SSD1306, Adafruit_GFX for debugging and signalling and if
supported by the MCU board or the hardware setup, the battery voltage can be managed by Adafruit_LC709203F.

## Structure

The code is structured in the following way:

- `TinyCon.ino` is the main entry point for the application, it deals with timing and basic Arduino setup.
- `TinyController.h/.cpp` are responsible for controller state management and infrastructure, making the
  basic connection between the building blocks and ensuring the correct blocks are active in each state.
- `GamepadController.h/.cpp` deals with all building blocks for the gamepad(s), allowing for up to 8 pads,
  but at the moment only using 2. It is further divided into `InputController.h/.cpp` to abstract possible
  sticks and buttons, `MPUController.h/.cpp` to abstract the IMU and `HapticController.h/.cpp` to abstract
  the haptic feedback controllers. These are using switches and unions instead of virtual functions to allow 
  the controller to own its driver and to avoid having to dependency-inject each driver separately for the
  limited scope of the project. To scale to other drivers, actual abstraction would be recommended.
- `Bluetooth.h/.cpp` deals with the Bluetooth state changes, including the advertising and connection handling.
- `USB.h/.cpp` deals with the USB state changes, including the USB HID gamepad handling, exposing haptics and MPU.
- `I2C.h/.cpp` deals with I2C access, providing command handling and a register file implementation.
- `CommandProcessor.h/.cpp` deals with handling commands that result from I2C, USB or Bluetooth communication,
  modifying available features and inserting haptic commands.
- `Indicators.h/.cpp` deals with the LED and OLED display, providing feedback on the current state of the controller.
- `Core/Drivers/Input/TITinyConTypes.h` contains the reusable definitions, that can be copied to another project to
  implement a driver for your project against.

Connections between the blocks are dependency-injected. All blocks are created on the stack to avoid problems
with dynamic allocation, and modern C++ features are used in some cases. Templates and excessive driver
implementations are avoided for now, it is up to you to insert your driver code into the existing structure if
you need support for additional hardware.

## Behavior 

 - If I2C, USB and Bluetooth are not connected, no updates are performed to save power.
 - I2C supersedes everything else, if connected, disable and disconnect Bluetooth and USB.
 - If USB is freshly connected, USB is enabled and Bluetooth is disconnected and disabled.
 - If USB is disconnected, Bluetooth is enabled and starts advertising for a limited time.
 - If USB is connected and the advertising button is depressed, or the select button remains
   down for 10s, force Bluetooth to advertise, but keep updating USB.
 - If Bluetooth stops advertising and is not connected, disable Bluetooth.
 - If Bluetooth successfully connects, and USB is connected, disable USB.
 - If Bluetooth disconnects while USB is connected, enable USB
 - If none of the above, we use watchdog sleep to save power and check for the select button
   to be down long enough (10s) to restart Bluetooth.

## Raw data reading

Most of the config data can be read without help, use this to read float16 values using a temporary python shell:

```python
import numpy as np
def floatv(v):
    return np.frombuffer(bytes([v & 0xff, v >> 8]), dtype=np.float16)[0]
floatv(0x3c00)
```

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.