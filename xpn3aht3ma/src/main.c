#include <windows.h>
#include <stddef.h>

#define IOCTL_XPN_PING CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

void main() {
	LPCWSTR deviceName = L"\\\\.\\Kmd_xpn_Link";
	HANDLE h = CreateFileW(deviceName, GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, 0, NULL);

	ULONG result = 0;
	DWORD bytesRet = 0;
	DeviceIoControl(h, IOCTL_XPN_PING, NULL, 0, &result, sizeof(result), &bytesRet, NULL);
}