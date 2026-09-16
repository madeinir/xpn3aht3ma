#include <ntddk.h>
#include <wdf.h>
#include <wdfcontrol.h>
#include "Public.h"
#include <Windows.h>

DRIVER_INITIALIZE DriverEntry;
void XpnEvtIoDeviceControl(WDFQUEUE Queue,
	WDFREQUEST Request,
	size_t OutputBufferLength,
	size_t InputBufferLength,
	ULONG IoControlCode);

NTSTATUS DriverEntry( PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath ) {
	//driver
	WDF_DRIVER_CONFIG config;
	NTSTATUS status;

	WDF_DRIVER_CONFIG_INIT(&config, WDF_NO_EVENT_CALLBACK);
	config.DriverInitFlags |= WdfDriverInitNonPnpDriver;

	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "XPN3AHT3MA: DriverEntry\n"));

	WDFDRIVER hDriver;
	status = WdfDriverCreate(DriverObject, RegistryPath, WDF_NO_OBJECT_ATTRIBUTES, &config, &hDriver);

	//device
	WDFDEVICE hDevice;

	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "XPN3AHT3MA: DeviceAdd\n"));

	UNICODE_STRING  symLinkName = { 0 };
	UNICODE_STRING  deviceFileName = { 0 };
	UNICODE_STRING securitySetting = { 0 };
	RtlInitUnicodeString(&symLinkName, L"\\DosDevices\\Kmd_xpn_Link");
	RtlInitUnicodeString(&deviceFileName, L"\\Device\\Kmd_xpn_Dev");
	RtlInitUnicodeString(&securitySetting, L"D:P(A;;GA;;;SY)(A;;GA;;;BA)");

	PWDFDEVICE_INIT DeviceInit = WdfControlDeviceInitAllocate(
		hDriver,
		&securitySetting
	);

	WdfDeviceInitAssignSDDLString(DeviceInit, &securitySetting);
	WdfDeviceInitSetCharacteristics(DeviceInit, FILE_DEVICE_SECURE_OPEN, FALSE);

	WdfDeviceInitAssignName(DeviceInit, &deviceFileName);

	status = WdfDeviceCreate(&DeviceInit, WDF_NO_OBJECT_ATTRIBUTES, &hDevice);
	if (!NT_SUCCESS(status)) {
		return status;
	}
	status = WdfDeviceCreateSymbolicLink(hDevice, &symLinkName);
	if (!NT_SUCCESS(status)) {
		return status;
	}

	//Очередь для обработки запросов из юзермода
	WDF_IO_QUEUE_CONFIG queueConfig;
	WDFQUEUE queue;

	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig, WdfIoQueueDispatchSequential);
	queueConfig.EvtIoDeviceControl = &XpnEvtIoDeviceControl;

	status = WdfIoQueueCreate(hDevice, &queueConfig, WDF_NO_OBJECT_ATTRIBUTES, &queue);
	if (!NT_SUCCESS(status)) {
		return status;
	}

	WdfControlFinishInitializing(hDevice);

	return status;
}

void XpnPinghandler
(
	WDFQUEUE Queue,
	WDFREQUEST Request,
	size_t OutputBufferLength,
	size_t InputBufferLength,
	ULONG IoControlCode
)
{
	UNREFERENCED_PARAMETER(Queue);
	UNREFERENCED_PARAMETER(InputBufferLength);
	UNREFERENCED_PARAMETER(IoControlCode);
	if (OutputBufferLength > 0)
	{
		void* buffer = NULL;
		size_t bufferLength = 0;

		NTSTATUS status = WdfRequestRetrieveOutputBuffer(Request, sizeof(ULONG), &buffer, &bufferLength);
		if (status == STATUS_SUCCESS)
		{
			*(PULONG)buffer = 0xdeadbeef;
		}
		WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, sizeof(ULONG));
	}
	else WdfRequestComplete(Request, STATUS_INVALID_PARAMETER);
}

void XpnEvtIoDeviceControl
(
	WDFQUEUE Queue,
	WDFREQUEST Request,
	size_t OutputBufferLength,
	size_t InputBufferLength,
	ULONG IoControlCode
)
{
	UNREFERENCED_PARAMETER(Queue);
	UNREFERENCED_PARAMETER(InputBufferLength);

	switch (IoControlCode)
	{
	case IOCTL_XPN_PING:
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "XPN3AHT3MA: ping received\n"));
		XpnPinghandler(Queue, Request, OutputBufferLength, InputBufferLength, IoControlCode);
		break;
	default:
		WdfRequestComplete(Request, STATUS_INVALID_DEVICE_REQUEST);
	}
}

