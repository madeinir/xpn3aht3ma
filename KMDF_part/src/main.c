#include "Public.h"
#include "xpnNetwork.c"
#include "xpnGUIDs.c"

DRIVER_INITIALIZE DriverEntry;

NTSTATUS RegisterWfp(PDEVICE_OBJECT deviceObject); 
HANDLE hEngine = NULL;

UINT32 calloutId = 0;

void XpnEvtIoDeviceControl(WDFQUEUE Queue,
	WDFREQUEST Request,
	size_t OutputBufferLength,
	size_t InputBufferLength,
	ULONG IoControlCode);

void xpnDriverUnload(WDFDRIVER Driver);

NTSTATUS DriverEntry( PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath ) {
	//Сам драйвер
	WDF_DRIVER_CONFIG config;
	NTSTATUS status;

	WDF_DRIVER_CONFIG_INIT(&config, WDF_NO_EVENT_CALLBACK);
	config.DriverInitFlags |= WdfDriverInitNonPnpDriver;
	config.EvtDriverUnload = xpnDriverUnload;

	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "XPN3AHT3MA: DriverEntry\n"));

	WDFDRIVER hDriver;
	status = WdfDriverCreate(DriverObject, RegistryPath, WDF_NO_OBJECT_ATTRIBUTES, &config, &hDriver);

	//Устройство для драйвера
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
	if (!NT_SUCCESS(status)) return status;
	status = WdfDeviceCreateSymbolicLink(hDevice, &symLinkName);
	if (!NT_SUCCESS(status)) return status;

	//Очередь для обработки запросов из юзермода
	WDF_IO_QUEUE_CONFIG queueConfig;
	WDFQUEUE queue;

	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig, WdfIoQueueDispatchSequential);
	queueConfig.EvtIoDeviceControl = &XpnEvtIoDeviceControl;

	status = WdfIoQueueCreate(hDevice, &queueConfig, WDF_NO_OBJECT_ATTRIBUTES, &queue);
	if (!NT_SUCCESS(status)) return status;

	WdfControlFinishInitializing(hDevice);

	PDEVICE_OBJECT deviceObject = WdfDeviceWdmGetDeviceObject(hDevice);
	status = RegisterWfp(deviceObject);
	if (!NT_SUCCESS(status)) return status;

	return status;
}

//регистрация поебни для WFP
NTSTATUS RegisterWfp(PDEVICE_OBJECT deviceObject) 
{

	FWPM_SESSION xpn_session = {0};
	xpn_session.flags = FWPM_SESSION_FLAG_DYNAMIC;
	NTSTATUS status = FwpmEngineOpen(NULL, RPC_C_AUTHN_WINNT, NULL, &xpn_session, &hEngine);
	if (!NT_SUCCESS(status)) return status;

	status = FwpmTransactionBegin0(hEngine, 0);
	if (!NT_SUCCESS(status)) return status;

	FWPM_PROVIDER0 provider = { 0 };
	provider.providerKey = PROVIDER_GUID;
	provider.displayData.name = L"XPN3AHT3MA_pro";
	provider.flags = 0;
	status = FwpmProviderAdd0(hEngine, &provider, NULL);
	if (!NT_SUCCESS(status)) return status;

	FWPM_SUBLAYER0 sublayer = { 0 };
	sublayer.subLayerKey = SUBLAYER_GUID;
	sublayer.displayData.name = L"XPN3AHT3MA_sub";
	sublayer.weight = 0x100;
	status = FwpmSubLayerAdd0(hEngine, &sublayer, NULL);
	if (!NT_SUCCESS(status)) return status;

	//калауты ебать их в рот
	FWPS_CALLOUT0 sCallout = { 0 };
	sCallout.calloutKey = CALLOUT_GUID;
	sCallout.classifyFn = XpnClassifyFn;
	sCallout.notifyFn = XpnNotifyFn;
	sCallout.flowDeleteFn = XpnFlowDeleteFn;
	status = FwpsCalloutRegister0(deviceObject, &sCallout, &calloutId);
	if (!NT_SUCCESS(status)) return status;

	FWPM_CALLOUT0 mCallout = { 0 };
	mCallout.calloutKey = CALLOUT_GUID;
	mCallout.displayData.name = L"XPN3AHT3MA_callout";
	mCallout.applicableLayer = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
	status = FwpmCalloutAdd0(hEngine, &mCallout, NULL, NULL);
	if (!NT_SUCCESS(status)) return status;

	FWPM_FILTER0 filter = { 0 };
	filter.displayData.name = L"XPN3AHT3MA_filter";
	filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
	filter.subLayerKey = SUBLAYER_GUID;
	filter.providerKey = &PROVIDER_GUID;
	filter.filterKey = FILTER_GUID;
	filter.action.type = FWP_ACTION_CALLOUT_TERMINATING;
	filter.action.calloutKey = CALLOUT_GUID;
	filter.weight.uint8 = 0x01;
	status = FwpmFilterAdd0(hEngine, &filter, NULL, 0);
	if (!NT_SUCCESS(status)) return status;

	status = FwpmTransactionCommit0(hEngine);
	if (!NT_SUCCESS(status)) return status;

	return STATUS_SUCCESS;
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

VOID xpnDriverUnload(WDFDRIVER Driver)
{
	UNREFERENCED_PARAMETER(Driver);
	FwpmFilterDeleteByKey0(hEngine, &FILTER_GUID);
	FwpmCalloutDeleteById0(hEngine, &CALLOUT_GUID);
	FwpsCalloutUnregisterById0(calloutId);
	FwpmSubLayerDeleteByKey0(hEngine, &SUBLAYER_GUID);
	FwpmProviderDeleteByKey0(hEngine, &PROVIDER_GUID);
	FwpmEngineClose0(hEngine);
	return;
}