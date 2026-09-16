import struct
import ctypes
from ctypes import wintypes

FILE_DEVICE_UNKNOWN = 0x00000022
METHOD_BUFFERED     = 0
FILE_ANY_ACCESS     = 0

def CTL_CODE(device_type, function, method, access):
    return (device_type << 16) | (access << 14) | (function << 2) | method

IOCTL_XPN_PING = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
print(hex(IOCTL_XPN_PING))  # 0x220E00

kernel32 = ctypes.WinDLL('kernel32', use_last_error=True)

GENERIC_READ  = 0x80000000
GENERIC_WRITE = 0x40000000
OPEN_EXISTING = 3
FILE_SHARE_READ  = 0x00000001
FILE_SHARE_WRITE = 0x00000002
INVALID_HANDLE_VALUE = wintypes.HANDLE(-1).value

# Пути к драйверам обычно вида \\.\XpnDevice
DEVICE_PATH = r'\\.\Kmd_xpn_Link'

kernel32.CreateFileW.restype = wintypes.HANDLE
kernel32.CreateFileW.argtypes = [
    wintypes.LPCWSTR, wintypes.DWORD, wintypes.DWORD,
    ctypes.c_void_p, wintypes.DWORD, wintypes.DWORD, wintypes.HANDLE
]

handle = kernel32.CreateFileW(
    DEVICE_PATH,
    GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    None,
    OPEN_EXISTING,
    0,
    None
)

if handle == INVALID_HANDLE_VALUE:
    raise ctypes.WinError(ctypes.get_last_error())

# Подготовка буферов
in_buffer = b''            # входные данные, если нужны (для METHOD_BUFFERED — любой bytes)
out_buffer = ctypes.create_string_buffer(64)  # буфер под ответ
bytes_returned = wintypes.DWORD(0)

kernel32.DeviceIoControl.argtypes = [
    wintypes.HANDLE, wintypes.DWORD,
    wintypes.LPCVOID, wintypes.DWORD,
    wintypes.LPVOID, wintypes.DWORD,
    ctypes.POINTER(wintypes.DWORD),
    wintypes.LPVOID
]
kernel32.DeviceIoControl.restype = wintypes.BOOL

result = kernel32.DeviceIoControl(
    handle,
    IOCTL_XPN_PING,
    in_buffer, len(in_buffer),
    out_buffer, ctypes.sizeof(out_buffer),
    ctypes.byref(bytes_returned),
    None
)

if not result:
    raise ctypes.WinError(ctypes.get_last_error())

print("Bytes returned:", bytes_returned.value)
value = struct.unpack('<I', out_buffer.raw[:4])[0]
print(hex(value))

kernel32.CloseHandle(handle)