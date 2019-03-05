#pragma once

#include <map>
#include <functional>

enum class DeviceEvent
{
    Arrival,
    RemoveComplete
};

class USBDeviceMonitor
{
public:
	USBDeviceMonitor();
	~USBDeviceMonitor();

	int StartMonitor();
	void StopMonitor();

    void AddNotifyFunction(const std::wstring& key, const std::function<void(DeviceEvent event, const wchar_t* devSymbolicLink)>& func);
    void RemoveNotifyFunction(const std::wstring& key);

private:
    void _NotifyDeviceChange(DeviceEvent event, const wchar_t* devSymbolicLink);

    static LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    std::map<std::wstring, std::function<void(DeviceEvent event, const wchar_t*)>> m_NotifyFuncMap;

    HWND m_MessageWindow = NULL;
    HDEVNOTIFY m_hDevNotify = NULL;
};

