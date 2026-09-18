#include "Public.h"

void XpnClassifyFn
(
	const FWPS_INCOMING_VALUES* inFixedValues,
	const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
	void* layerData,
	const void* classifyContext,
	const FWPS_FILTER* filter,
	UINT64 flowContext,
	FWPS_CLASSIFY_OUT* classifyOut)
{
	int pid = 0;
	if (inMetaValues->currentMetadataValues & FWPS_METADATA_FIELD_PROCESS_ID) pid = inMetaValues->processId;
	UINT32 dstAddr = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_REMOTE_ADDRESS].value.int32;
	UINT16 dstPort = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_REMOTE_PORT].value.int16;
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "XPN3AHT3MA: Outbound connection from PID %d to %lu:%d\n", pid, dstAddr, dstPort));
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "XPN3AHT3MA: rights=0x%x\n", classifyOut->rights));
	classifyOut->actionType = FWP_ACTION_BLOCK;
}

NTSTATUS XpnNotifyFn
(
	FWPS_CALLOUT_NOTIFY_TYPE notifyType,
	const GUID* filterKey,
	const FWPS_FILTER* filter)
{
	return STATUS_SUCCESS;
}

void XpnFlowDeleteFn
(
	UINT16 layerId,
	UINT32 calloutId,
	UINT64 flowContext)
{

}