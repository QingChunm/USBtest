#include "Misc\stdafx.h"
#include "DeviceManager.h"

#include <initguid.h>
#include <devpkey.h>

#include <Cfgmgr32.h>
#include <Wiaintfc.h>

#include <devguid.h>
#include <winioctl.h>
#include <rpc.h>

#include <algorithm>

#include <vector>

#include "../Data/AX327X.h"


DeviceManager::DeviceManager()
{
}


DeviceManager::~DeviceManager()
{
}

void DeviceManager::Initialize()
{
    if (m_DeviceInfoSet != NULL)
    {
        return;
    }

    m_DeviceInfoSet = SetupDiCreateDeviceInfoList(NULL, NULL);

    m_usbDevMonitor.AddNotifyFunction(L"DevMgr",
        [this](DeviceEvent event, const wchar_t* devSymbolicLink) -> void{
        this->_OnUsbEvent(event, devSymbolicLink);
    }
    );
    m_usbDevMonitor.StartMonitor();
}

void DeviceManager::Uninitialize()
{
    m_usbDevMonitor.StopMonitor();
    m_usbDevMonitor.RemoveNotifyFunction(L"DevMgr");

    SetupDiDestroyDeviceInfoList(m_DeviceInfoSet);
}

std::map<std::wstring, std::wstring> DeviceManager::m_SupportedHardwareSpecificsMap = { 
    { L"BuildWinDiskInterface", L"Buildwin" },
    { L"BuildWin", L"VID_1908" },
    { L"iPhoneForTest", L"VID_05AC&PID_12A8" }
};


void DeviceManager::_OnUsbEvent(DeviceEvent event, const wchar_t* devSymbolicLink)
{
    switch (event)
    {
    case DeviceEvent::Arrival:
    {
        SP_DEVINFO_DATA devInfoData = _GetDeviceInfo(devSymbolicLink);
        if (_IsSupportedDevice(devInfoData))
        {
            _AddDevice(devSymbolicLink, devInfoData);
        }
        break;
    }
    case DeviceEvent::RemoveComplete:
        _RemoveDevice(devSymbolicLink);
        break;
    default:
        break;
    }
}

void DeviceManager::AddSupportedHardwareId(const wchar_t* deviceModel, const wchar_t* hardwareId)
{
    m_SupportedHardwareSpecificsMap.insert(std::make_pair(deviceModel, hardwareId));
}

void DeviceManager::RemoveSupportedHardwareId(const wchar_t* deviceModel)
{
    m_SupportedHardwareSpecificsMap.erase(deviceModel);
}

bool DeviceManager::_IsSupportedDevice(SP_DEVINFO_DATA devInfoData)
{
    return _IsSupportedDevice(devInfoData, m_DeviceInfoSet);
}

bool DeviceManager::_IsSupportedDevice(SP_DEVINFO_DATA devInfoData, HDEVINFO deviceInfoSet)
{
    std::wstring devHardwareId = _GetDeviceHardwareId(devInfoData, deviceInfoSet);
    int a = 1;
    for each (auto item in m_SupportedHardwareSpecificsMap)
    {
        if (devHardwareId.find(item.second) != std::wstring::npos)
        {
            return true;
        }
    }

    return false;
}

AX32XXDevice* DeviceManager::_AddDevice(const wchar_t* devSymbolicLink, SP_DEVINFO_DATA devInfoData)
{
    wchar_t uvcInterfaceName[MAX_PATH] = L"";

    DEVINST parentDevInst = 0;
    DEVINST tmpDevInst = devInfoData.DevInst;

    CONFIGRET result = CR_FAILURE;
    DEVPROPTYPE tmpPropType = 0;

    std::wstring parentDesc;
    unsigned long tmpBufferSize = 500;
    parentDesc.resize(tmpBufferSize);

    do
    {
        result = CM_Get_Parent(&parentDevInst, tmpDevInst, NULL);
        if (result != CR_SUCCESS)
        {
            break;
        }

		GUID parentGuid;
		CM_Get_DevNode_Property(parentDevInst, &DEVPKEY_Device_ClassGuid, &tmpPropType, (PBYTE)&parentGuid, &tmpBufferSize, 0);
		if (parentGuid != GUID_DEVCLASS_USB)
		{
			break;
		}

        CM_Get_DevNode_Property(parentDevInst, &DEVPKEY_Device_DeviceDesc, &tmpPropType, (PBYTE)&parentDesc[0], &tmpBufferSize, 0);
        parentDesc.resize(tmpBufferSize / sizeof(wchar_t) - 1);

        CM_Get_DevNode_Property(parentDevInst, &DEVPKEY_Device_DeviceDesc, &tmpPropType, (PBYTE)&parentDesc[0], &tmpBufferSize, 0);
        if (parentDesc == L"USB Composite Device")
        {
            break;
        }
        tmpDevInst = parentDevInst;
    } while (true);

    
    wchar_t devLocation[MAX_PATH] = { 0 };
    tmpBufferSize = sizeof(devLocation);
    result = CM_Get_DevNode_Property(parentDevInst, &DEVPKEY_Device_LocationInfo, &tmpPropType, (PBYTE)devLocation, &tmpBufferSize, 0);
    if (result != CR_SUCCESS || _tcslen(devLocation) == 0)
    {
        return nullptr;
    }

    DEVINST childDevInst = NULL;
    CM_Get_Child(&childDevInst, parentDevInst, NULL);

    if (childDevInst == 0)
    {
        return nullptr;
    }

    while (true)
    {
        GUID guid, guid2;

        tmpBufferSize = sizeof(guid);
        result = CM_Get_DevNode_Property(childDevInst, &DEVPKEY_Device_ClassGuid, &tmpPropType, (PBYTE)&guid,
            (PULONG)&tmpBufferSize, 0);

        // win10的cammera
        auto uuid2Str = L"CA3E7AB9-B4C3-4AE6-8251-579EF933890F";

        UuidFromString((RPC_WSTR)uuid2Str, &guid2);

        // image接口，用于UVC
        if (guid == GUID_DEVINTERFACE_IMAGE || guid == guid2)
        {
            tmpBufferSize = sizeof(uvcInterfaceName);
            CM_Get_DevNode_Property(childDevInst, &DEVPKEY_Device_FriendlyName, &tmpPropType, (PBYTE)uvcInterfaceName,
                (PULONG)&tmpBufferSize, 0);
        }

        DEVINST nextChildDevInst = 0;
        if (CM_Get_Sibling(&nextChildDevInst, childDevInst, 0) == CR_SUCCESS)
        {
            childDevInst = nextChildDevInst;
        }
        else
        {
            break;
        }
    }

    if (_tcslen(uvcInterfaceName) == 0)
    {
        return nullptr;
    }

    AX32XXDevice* aX32XXDevice = new AX327X(devSymbolicLink, uvcInterfaceName, devLocation);
    aX32XXDevice->InitDebugParam();

    std::wstring devicePath = devSymbolicLink;
    std::transform(devicePath.begin(), devicePath.end(), devicePath.begin(), ::toupper);

    m_Ax32xxDevMap.insert(std::make_pair(devicePath, aX32XXDevice));

    m_deviceChangeNotifyFunc((int)DeviceEvent::Arrival, devLocation, L"AX327X", uvcInterfaceName);

    return aX32XXDevice;
}

void DeviceManager::_RemoveDevice(const wchar_t* devSymbolicLink)
{
    std::wstring devicePath = devSymbolicLink;
    std::transform(devicePath.begin(), devicePath.end(), devicePath.begin(), ::toupper);

    auto ite = m_Ax32xxDevMap.find(devicePath);
    if (ite != m_Ax32xxDevMap.end())
    {
        auto item = ite->second;

        m_deviceChangeNotifyFunc((int)DeviceEvent::RemoveComplete, item->GetDevLocation(), L"AX327X", L"");
        m_Ax32xxDevMap.erase(devicePath);

        delete item;
    }
}

std::wstring DeviceManager::_GetDeviceHardwareId(SP_DEVINFO_DATA devInfoData)
{
    return _GetDeviceHardwareId(devInfoData, m_DeviceInfoSet);
}

std::wstring DeviceManager::_GetDeviceHardwareId(SP_DEVINFO_DATA devInfoData, HDEVINFO deviceInfoSet)
{
    DEVPROPTYPE propType = 0;
    DWORD bufferSize = 0;
    std::wstring devHardwareIds = L"";
    devHardwareIds.resize(1000);
    SetupDiGetDeviceProperty(deviceInfoSet, &devInfoData, &DEVPKEY_Device_HardwareIds, &propType,
        reinterpret_cast<PBYTE>(&devHardwareIds[0]), devHardwareIds.size(), &bufferSize, 0);

    return devHardwareIds;
}

SP_DEVINFO_DATA DeviceManager::_GetDeviceInfo(const wchar_t* devSymbolicLink)
{
    return _GetDeviceInfo(devSymbolicLink, m_DeviceInfoSet);
}

SP_DEVINFO_DATA DeviceManager::_GetDeviceInfo(const wchar_t* devSymbolicLink, HDEVINFO deviceInfoSet)
{
    SP_DEVICE_INTERFACE_DATA interfaceData = { 0 };
    interfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    SetupDiOpenDeviceInterface(deviceInfoSet, devSymbolicLink, 0, &interfaceData);

    wchar_t devInstanceId[MAX_PATH] = { 0 };
    DEVPROPTYPE propType = 0;
    DWORD bufferSize = 0;

    SetupDiGetDeviceInterfaceProperty(deviceInfoSet, &interfaceData, &DEVPKEY_Device_InstanceId, &propType,
        reinterpret_cast<PBYTE>(devInstanceId), sizeof(devInstanceId), &bufferSize, 0);

    SP_DEVINFO_DATA devInfoData = { 0 };
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    SetupDiOpenDeviceInfo(deviceInfoSet, devInstanceId, NULL, DIOD_INHERIT_CLASSDRVS, &devInfoData);

    return devInfoData;
}

AX32XXDevice* DeviceManager::GetDevice(const wchar_t* devLocation)
{
    for each (auto item in m_Ax32xxDevMap)
    {
        if (_tcscmp(item.second->GetDevLocation(), devLocation) == 0)
        {
            return item.second;
        }
    }

    return nullptr;
}

DeviceManager& DeviceManager::GetInstance()
{
    static DeviceManager instance;
    return instance;
}

void DeviceManager::AddDevChangeNotifyFunc(std::function<void(int, const wchar_t*, const wchar_t*, const wchar_t*)> deviceChangeNotifyFunc)
{
    m_deviceChangeNotifyFunc = deviceChangeNotifyFunc;
}

void DeviceManager::RemoveDevChangeNotifyFunc()
{
    m_deviceChangeNotifyFunc = nullptr;
}

void DeviceManager::ScanDevice()
{
    HDEVINFO hardware_dev_info_set = SetupDiGetClassDevs(&GUID_DEVINTERFACE_DISK, NULL, NULL, DIGCF_DEVICEINTERFACE|DIGCF_PRESENT);

    int devidx = 0;
    while (INVALID_HANDLE_VALUE != hardware_dev_info_set)
    {
        SP_DEVINFO_DATA devInfoData;
        devInfoData.cbSize = sizeof(devInfoData);

        if (SetupDiEnumDeviceInfo(hardware_dev_info_set, devidx, &devInfoData))
        {
            if (_IsSupportedDevice(devInfoData, hardware_dev_info_set))
            {
                SP_DEVICE_INTERFACE_DATA devInterfaceData;
                devInterfaceData.cbSize = sizeof(devInterfaceData);

                SetupDiEnumDeviceInterfaces(hardware_dev_info_set, nullptr, &GUID_DEVINTERFACE_DISK, devidx, &devInterfaceData);

                DWORD interfaceDetailSize = 0;
                SetupDiGetInterfaceDeviceDetail(hardware_dev_info_set, &devInterfaceData, nullptr, interfaceDetailSize,
                    &interfaceDetailSize, nullptr);

                PSP_DEVICE_INTERFACE_DETAIL_DATA pDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)new char[interfaceDetailSize];
                pDetailData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
                SetupDiGetInterfaceDeviceDetail(hardware_dev_info_set, &devInterfaceData, pDetailData, interfaceDetailSize,
                    &interfaceDetailSize, nullptr);

                auto device = _AddDevice(pDetailData->DevicePath, devInfoData);
            }
            devidx++;
            continue;
        }
        else
        {
            break;
        }
    }

    if (INVALID_HANDLE_VALUE != hardware_dev_info_set) 
    {
        SetupDiDestroyDeviceInfoList(hardware_dev_info_set);
    }
}
