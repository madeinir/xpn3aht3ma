#pragma once

#include <ntddk.h>
#define NDIS_SUPPORT_NDIS6 1
#include <ndis.h> 
#include <wdf.h>
#include <fwpmk.h>
#include <fwpsk.h>
#include <wdfcontrol.h>


#define IOCTL_XPN_PING \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_XPN_LISTEN \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

extern const GUID PROVIDER_GUID;
extern const GUID SUBLAYER_GUID;
extern const GUID CALLOUT_GUID;
extern const GUID FILTER_GUID;
extern const GUID IN_CALLOUT_GUID;