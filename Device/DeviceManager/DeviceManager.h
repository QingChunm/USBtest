#pragma once

#include "USBDeviceMonitor.h"

#include <map>
#include <string>

#include <SetupAPI.h>

#include "Data/AX32XXDevice.h"


class DeviceManager
{
public:

    static DeviceManager& GetInstance();

    void Initialize();
    void Uninitialize();

    void AddSupportedHardwareId(const wchar_t* deviceModel, const wchar_t* hardwareId);
    void RemoveSupportedHardwareId(const wchar_t* deviceModel);

    void AddDevChangeNotifyFunc(std::function<void(int,const wchar_t*, const wchar_t*, const wchar_t*)> deviceChangeNotifyFunc);
    void RemoveDevChangeNotifyFunc();

    void ScanDevice();

    AX32XXDevice* GetDevice(const wchar_t* devLocation);

private:
    DeviceManager();
    ~DeviceManager();

    AX32XXDevice* _AddDevice(const wchar_t* devSymbolicLink, SP_DEVINFO_DATA devInfoData);
    void _RemoveDevice(const wchar_t* devSymbolicLink);

    bool _IsSupportedDevice(SP_DEVINFO_DATA devInfoData);
    bool _IsSupportedDevice(SP_DEVINFO_DATA devInfoData, HDEVINFO deviceInfoSet);
    std::wstring _GetDeviceHardwareId(SP_DEVINFO_DATA devInfoData);
    std::wstring _GetDeviceHardwareId(SP_DEVINFO_DATA devInfoData, HDEVINFO deviceInfoSet);
    SP_DEVINFO_DATA _GetDeviceInfo(const wchar_t* devSymbolicLink);
    SP_DEVINFO_DATA _GetDeviceInfo(const wchar_t* devSymbolicLink, HDEVINFO deviceInfoSet);
    void _OnUsbEvent(DeviceEvent event, const wchar_t* devSymbolicLink);

    HDEVINFO m_DeviceInfoSet;
    USBDeviceMonitor m_usbDevMonitor;

    std::map<std::wstring, AX32XXDevice*> m_Ax32xxDevMap;
    std::function<void(int event,
        const wchar_t* devLocation, const wchar_t* devModel, const wchar_t* uvcInterfaceName)> m_deviceChangeNotifyFunc;

    static std::map<std::wstring, std::wstring> m_SupportedHardwareSpecificsMap;
};

