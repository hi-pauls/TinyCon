#pragma once

#include "Config.h"
#include "GamepadController.h"
#include "CommandProcessor.h"

#include <Arduino.h>

#include <functional>

#if NO_USB
class USBController
{
public:
    USBController(GamepadController&, CommandProcessor&) {}

    void Init() {}
    void Update() {}
    void Disabled() {}
    bool Mounted() { return false; }
};
#else
#include <Adafruit_TinyUSB.h>

namespace TinyCon
{
class USBController
{
public:
    enum Reports : uint8_t
    {
        ReportGamepad = 1,
        ReportMpu = 2,
        ReportCommand = 3
    };

private:
    static constexpr int16_t MpuReportSize = 21 * GamepadController::MaxControllers;
    static constexpr uint8_t HidDescriptor[] =
        {
            TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(ReportGamepad)),
            TUD_HID_REPORT_DESC_GENERIC_INOUT(MpuReportSize, HID_REPORT_ID(ReportMpu)),
            TUD_HID_REPORT_DESC_GENERIC_INOUT(14, HID_REPORT_ID(ReportCommand))
        };

public:
    USBController(const GamepadController& controller, CommandProcessor& processor) : Controller(controller), Processor(processor) {}

    void Init();
    void Update();
    void SetActive(bool active) { Active = active; }
    [[nodiscard]] bool IsActive() const { return Active; }
    [[nodiscard]] bool IsConnected() const { return Connected; }

private:
    static std::function<void(uint8_t, hid_report_type_t, const uint8_t*, uint16_t)> ReportReceived;
    static std::function<uint16_t(uint8_t, hid_report_type_t, uint8_t*, uint16_t)> ReportRequested;
    static void UsbHidReportReceived(uint8_t reportId, hid_report_type_t report, const uint8_t* data, uint16_t length);
    static uint16_t UsbHidReportRequested(uint8_t reportId, hid_report_type_t report, uint8_t* data, uint16_t length);

    bool Active = false;
    bool Connected = false;
    Adafruit_USBD_HID Gamepad;

    const GamepadController& Controller;
    CommandProcessor& Processor;
};
#endif
}