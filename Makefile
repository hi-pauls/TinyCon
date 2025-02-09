MCU_BOARD ?= feather-nrf52840
#MCU_BOARD ?= feather-esp32s3
TINYCON_VENDOR := Hel Industries
TINYCON_PRODUCT := TinyCon
USB_VENDOR := $(TINYCON_VENDOR)
USB_PRODUCT := $(TINYCON_PRODUCT) USB
CPPFLAGS += -O0 -g "-DTINYCON_VENDOR=\"$(TINYCON_VENDOR)\"" "-DTINYCON_PRODUCT=\"$(TINYCON_PRODUCT)\""
CXXFLAGS += -std=gnu++17
#VERBOSE = 1

include $(patsubst %/,%,$(abspath $(dir $(firstword $(MAKEFILE_LIST)))))/../../Toolchain/BuildSystem/Arduino/ArduinoFlags.mk

ARDUINO_LIBS_PATH := $(PROJECT_PATH)/../Arduino/libraries
MODULES += Wire:$(CORE_LIB_PATH)/Wire
MODULES += SPI:$(CORE_LIB_PATH)/SPI
MODULES += Adafruit_TinyUSB:$(CORE_LIB_PATH)/Adafruit_TinyUSB_Arduino
MODULES += Adafruit_LittleFS:$(CORE_LIB_PATH)/Adafruit_LittleFS
MODULES += InternalFileSystem:$(CORE_LIB_PATH)/InternalFileSytem
MODULES += Bluefruit52Lib:$(CORE_LIB_PATH)/Bluefruit52Lib
MODULES += Adafruit_GFX:$(ARDUINO_LIBS_PATH)/Adafruit_GFX_Library
MODULES += Adafruit_BusIO:$(ARDUINO_LIBS_PATH)/Adafruit_BusIO
MODULES += Adafruit_Sensor:$(ARDUINO_LIBS_PATH)/Adafruit_Sensor
MODULES += Adafruit_SSD1306:$(ARDUINO_LIBS_PATH)/Adafruit_SSD1306
MODULES += Adafruit_LC709203F:$(ARDUINO_LIBS_PATH)/Adafruit_LC709203F
MODULES += Adafruit_seesaw:$(ARDUINO_LIBS_PATH)/Adafruit_seesaw_Library
MODULES += Adafruit_ICM20X:$(ARDUINO_LIBS_PATH)/Adafruit_ICM20X
MODULES += Adafruit_NeoPixel:$(ARDUINO_LIBS_PATH)/Adafruit_NeoPixel
MODULES += SoftWire:$(ARDUINO_LIBS_PATH)/SoftWire
MODULES += AsyncDelay:$(ARDUINO_LIBS_PATH)/AsyncDelay
MODULES += Adafruit_SleepyDog:$(ARDUINO_LIBS_PATH)/Adafruit_SleepyDog

include $(ARDUINO_BUILD_SYSTEM_PATH)/ArduinoTargets.mk
