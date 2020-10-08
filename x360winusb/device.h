#pragma once
//
// Define below GUIDs
//
#include <Windows.h>
#include <winusb.h>
#include <initguid.h>

//
// Device Interface GUID.
// Used by all WinUsb devices that this application talks to.
// Must match "DeviceInterfaceGUIDs" registry value specified in the INF file.
// 2420afbf-8872-4121-bcc9-6dbff8c1a1eb
//
//DEFINE_GUID(GUID_DEVINTERFACE_USBApplication1, 0x2420afbf, 0x8872, 0x4121, 0xbc, 0xc9, 0x6d, 0xbf, 0xf8, 0xc1, 0xa1, 0xeb);
  DEFINE_GUID(GUID_DEVINTERFACE_USBApplication1, 0x05889cbc, 0xa9a6, 0x4684, 0xba, 0xb9, 0xd2, 0xb4, 0x6c, 0xb0, 0x04, 0xd0);
    //{05889CBC-A9A6-4684-BAB9-D2B46CB004D0}

typedef struct _DEVICE_DATA {

    BOOL                    HandlesOpen;
    WINUSB_INTERFACE_HANDLE WinusbHandle;
    HANDLE                  DeviceHandle;
    TCHAR                   DevicePath[MAX_PATH];

} DEVICE_DATA, *PDEVICE_DATA;


struct PIPE_ID
{
    UCHAR  PipeInId;
    UCHAR  PipeOutId;
};



HRESULT
OpenDevice(
    _Out_     PDEVICE_DATA DeviceData,
    _Out_opt_ PBOOL        FailureDeviceNotFound
    );

VOID
CloseDevice(
    _Inout_ PDEVICE_DATA DeviceData
    );

BOOL QueryDeviceEndpoints(WINUSB_INTERFACE_HANDLE hDeviceHandle, PIPE_ID* pipeid);