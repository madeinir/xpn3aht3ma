#include <ntddk.h>
#include <wdf.h>

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD XpnAdd;

NTSTATUS DriverEntry( PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath ) {

	WDF_DRIVER_CONFIG config;
	NTSTATUS status;

	WDF_DRIVER_CONFIG_INIT(&config, XpnAdd);

	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "xpn: DriverEntry\n"));

	status = WdfDriverCreate(DriverObject, RegistryPath, WDF_NO_OBJECT_ATTRIBUTES, &config, WDF_NO_HANDLE);

	return status;
}

NTSTATUS XpnAdd( WDFDRIVER Driver, PWDFDEVICE_INIT DeviceInit ) {
	UNREFERENCED_PARAMETER(Driver);
	
	NTSTATUS status;
	WDFDEVICE hDevice;

	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "xpn: DeviceAdd\n"));

	UNICODE_STRING  symLinkName = { 0 };
	UNICODE_STRING  deviceFileName = { 0 };
	UNICODE_STRING securitySetting = { 0 };
	RtlInitUnicodeString(&symLinkName, L"\\DosDevice\\Kmd_xpn_Link");
	RtlInitUnicodeString(&deviceFileName, L"\\Device\\Kmd_xpn_Dev");
	RtlInitUnicodeString(&securitySetting, L"D:P(A;;GA;;;SY)(A;;GA;;;BA)");
	
	WdfDeviceInitAssignSDDLString(DeviceInit, &securitySetting);
	WdfDeviceInitSetCharacteristics(DeviceInit, FILE_DEVICE_SECURE_OPEN, FALSE);

	WdfDeviceInitAssignName(DeviceInit, &deviceFileName);

	status = WdfDeviceCreate(&DeviceInit, WDF_NO_OBJECT_ATTRIBUTES, &hDevice);
	WdfDeviceCreateSymbolicLink(hDevice, &symLinkName);

	return status;
}