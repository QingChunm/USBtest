#include "Misc\stdafx.h"
#include "USBDeviceMonitor.h"

#include <Dbt.h>

#include <initguid.h>
#include <winioctl.h>



USBDeviceMonitor::USBDeviceMonitor()
{
}


USBDeviceMonitor::~USBDeviceMonitor()
{
}

int USBDeviceMonitor::StartMonitor()
{
    WNDCLASS windowClass = {};
    windowClass.lpfnWndProc = USBDeviceMonitor::WindowProcedure;
    LPCWSTR windowClassName = L"MessageOnlyWindow";
    windowClass.lpszClassName = windowClassName;

    if (!RegisterClass(&windowClass))
    {
        return false;
    }

    m_MessageWindow = CreateWindow(windowClass.lpszClassName, NULL, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL);

    if (m_MessageWindow == INVALID_HANDLE_VALUE)
    {
        return -1;
    }

    ::SetWindowLongPtr(m_MessageWindow, GWLP_USERDATA, reinterpret_cast<LPARAM>(this));

    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

    ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = GUID_DEVINTERFACE_DISK;

    m_hDevNotify = RegisterDeviceNotification(m_MessageWindow, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
    if (!m_hDevNotify) 
    {
        return -2;
    }

    return 0;
}

void USBDeviceMonitor::StopMonitor()
{
    UnregisterDeviceNotification(m_hDevNotify);
}

LRESULT CALLBACK USBDeviceMonitor::WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_DEVICECHANGE)
    {
        USBDeviceMonitor* pMonitorInstance = reinterpret_cast<USBDeviceMonitor*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
        PDEV_BROADCAST_DEVICEINTERFACE pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;

        switch (wParam)
        {
        case DBT_DEVICEARRIVAL:
            pMonitorInstance->_NotifyDeviceChange(DeviceEvent::Arrival, pDevInf->dbcc_name);
            break;
        case DBT_DEVICEREMOVECOMPLETE:
            pMonitorInstance->_NotifyDeviceChange(DeviceEvent::RemoveComplete, pDevInf->dbcc_name);
            break;

        default:
            break;
        }

        return true;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void USBDeviceMonitor::_NotifyDeviceChange(DeviceEvent event, const wchar_t* devSymbolicLink)
{
    for each (auto item in m_NotifyFuncMap)
    {
        auto func = item.second;
        func(event, devSymbolicLink);
    }
}

void USBDeviceMonitor::AddNotifyFunction(const std::wstring& key, const std::function<void(DeviceEvent event, const wchar_t* devSymbolicLink)>& func)
{
    m_NotifyFuncMap.insert(std::make_pair(key, func));
}

void USBDeviceMonitor::RemoveNotifyFunction(const std::wstring& key)
{
    m_NotifyFuncMap.erase(key);
}
