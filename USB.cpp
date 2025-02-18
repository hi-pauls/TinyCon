#include "USB.h"
using LogUsb = Tiny::TILogTarget<TinyCon::UsbLogLevel>;

#if !NO_USB
void USBController::UsbHidReportReceived(uint8_t reportId, hid_report_type_t report, const uint8_t* data, uint16_t length)
{ ReportReceived(reportId, report, data, length); }
uint16_t USBController::UsbHidReportRequested(uint8_t reportId, hid_report_type_t report, uint8_t* data, uint16_t length)
{ return ReportRequested(reportId, report, data, length); }

std::function<void(uint8_t, hid_report_type_t, const uint8_t*, uint16_t)> USBController::ReportReceived =
    [](uint8_t reportId, hid_report_type_t report, const uint8_t* data, uint16_t length) {};
std::function<uint16_t(uint8_t, hid_report_type_t, uint8_t*, uint16_t)> USBController::ReportRequested =
    [](uint8_t reportId, hid_report_type_t report, uint8_t* data, uint16_t length) { return 0; };

void USBController::Init()
{
    LogUsb::Info("USB Init", Tiny::TIEndl);
    Gamepad.setPollInterval(10);
    Gamepad.setStringDescriptor("Game Controller");
    Gamepad.setReportDescriptor(HidDescriptor, sizeof(HidDescriptor));

    ReportReceived = [this](uint8_t reportId, hid_report_type_t report, const uint8_t* data, uint16_t length)
        {
            if (Processor.GetUSBEnabled() && reportId == USBController::ReportCommand && length >= 1)
                Processor.ProcessCommand({data, length});
        };
    Gamepad.setReportCallback(UsbHidReportRequested, UsbHidReportReceived);
    Gamepad.begin();
}

void USBController::Update()
{
    LogUsb::Debug("USB: ");
    if (!Processor.GetUSBEnabled() || !Active)
    {
        if (!Processor.GetUSBEnabled()) LogUsb::Debug("Disabled");
        else LogUsb::Debug("Inactive");

        Active = false;
        Connected = false;
    }
    else if ((Connected = TinyUSBDevice.mounted() && Gamepad.ready()))
    {
        LogUsb::Debug("Controller");
        auto report = Controller.MakeHidReport();
        Gamepad.sendReport(ReportGamepad, &report, sizeof(report));

        LogUsb::Debug(", MPU");
        uint8_t data[MpuReportSize];
        auto size = Controller.MakeMpuBuffer({data, 42});
        Gamepad.sendReport(ReportMpu, data, size);
    }

    LogUsb::Info(Tiny::TIEndl);
}
#endif