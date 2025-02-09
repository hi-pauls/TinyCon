#include "USB.h"

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
    LOG_USB_LN("USB Init");
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
    LOG_USB("USB: ");
    if (!Processor.GetUSBEnabled() || !Active)
    {
        if (!Processor.GetUSBEnabled()) LOG_USB("Disabled");
        else LOG_USB("Inactive");

        Active = false;
        Connected = false;
    }
    else if ((Connected = TinyUSBDevice.mounted() && Gamepad.ready()))
    {
        LOG_USB("Controller");
        auto report = Controller.MakeHidReport();
        Gamepad.sendReport(ReportGamepad, &report, sizeof(report));

        LOG_USB(", MPU");
        uint8_t data[MpuReportSize];
        auto size = Controller.MakeMpuBuffer({data, 42});
        Gamepad.sendReport(ReportMpu, data, size);
    }

    LOG_USB_LN();
}
#endif