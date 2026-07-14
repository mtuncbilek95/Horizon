#include "Win32ErrorHelpers.h"

namespace Horizon::PAL
{
	std::string Win32ErrorHelpers::GetLastErrorString(DWORD errCode)
	{
		LPSTR buf = NULL;
		DWORD len = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buf, 0, NULL);

		if (len == 0 || buf == nullptr)
			return "Unknown error (" + std::to_string(errCode) + ")";

		std::string msg(buf, len);
		LocalFree(buf);

		while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r'))
			msg.pop_back();

		return msg;
	}
}