#include <Runtime/PAL/Hardware/Monitor.h>

#include <Windows.h>
#include <ShellScalingApi.h>

namespace Horizon::PAL
{
	namespace
	{
		BOOL CALLBACK EnumMonitorProc(HMONITOR hMonitor, HDC, LPRECT, LPARAM userData)
		{
			List<MonitorInfo>& monitors = *(List<MonitorInfo>*)userData;

			MONITORINFO info = {};
			info.cbSize = sizeof(info);

			if (!GetMonitorInfo(hMonitor, &info))
				return TRUE;

			UINT dpiX = 96, dpiY = 96;
			if (FAILED(GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY)))
				dpiX = 96;

			MonitorInfo monitor = {};
			monitor.posX = info.rcMonitor.left;
			monitor.posY = info.rcMonitor.top;
			monitor.width = u32(info.rcMonitor.right - info.rcMonitor.left);
			monitor.height = u32(info.rcMonitor.bottom - info.rcMonitor.top);
			monitor.workPosX = info.rcWork.left;
			monitor.workPosY = info.rcWork.top;
			monitor.workWidth = u32(info.rcWork.right - info.rcWork.left);
			monitor.workHeight = u32(info.rcWork.bottom - info.rcWork.top);
			monitor.dpiScale = f32(dpiX) / 96.0f;
			monitor.isPrimary = (info.dwFlags & MONITORINFOF_PRIMARY) != 0;

			if (monitor.isPrimary)
				monitors.PushAt(0, monitor);
			else
				monitors.PushBack(monitor);

			return TRUE;
		}
	}

	List<MonitorInfo> Monitor::EnumerateMonitors()
	{
		List<MonitorInfo> monitors;
		EnumDisplayMonitors(nullptr, nullptr, EnumMonitorProc, (LPARAM)&monitors);

		return monitors;
	}
}