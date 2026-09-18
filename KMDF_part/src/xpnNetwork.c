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

}

void XpnNotifyFn
(
	FWPS_CALLOUT_NOTIFY_TYPE notifyType,
	const GUID* filterKey,
	const FWPS_FILTER* filter)
{

}

void XpnFlowDeleteFn
(
	UINT16 layerId,
	UINT32 calloutId,
	UINT64 flowContext)
{

}