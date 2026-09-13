#include <windows.h>
#include <stddef.h>
#include "../../KMDF_part/src/Public.h"

void main() {
	LPCWSTR deviceName = L"\\\\.\\Kmd_xpn_Link";
	HANDLE h = CreateFileW(deviceName, GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, 0, NULL);

	ULONG result = 0;
	DWORD bytesRet = 0;
	DeviceIoControl(h, IOCTL_XPN_PING, NULL, 0, &result, sizeof(result), &bytesRet, NULL);
}