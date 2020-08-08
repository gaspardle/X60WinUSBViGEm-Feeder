#include "pch.h"

#include <stdio.h>
#include <cstdint>
#include "main.h"
#include <iostream>



BOOL ReadFromBulkEndpoint(WINUSB_INTERFACE_HANDLE hDeviceHandle, UCHAR pID, ULONG cbSize)
{
    if (hDeviceHandle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }
    //WinUsb_SetPipePolicy(hDeviceHandle, pID, PIPE_TRANSFER_TIMEOUT, sizeof(ULONG), 500);

    BOOL bResult = TRUE;
    UCHAR* szBuffer = (UCHAR*)LocalAlloc(LPTR, sizeof(UCHAR) * cbSize);
   
    ULONG cbRead = 0;
    

    OVERLAPPED overlapped;
    ZeroMemory(&overlapped, sizeof(overlapped));
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);


    bResult = WinUsb_ReadPipe(hDeviceHandle, pID, szBuffer, cbSize, &cbRead, 0);

    //WinUsb_GetOverlappedResult(hDeviceHandle, &overlapped, &cbRead, TRUE);
    if (!bResult)
    {
        goto done;
    }

    
    printf("pipe %d: len: %d : ", pID, cbRead);
    for (ULONG i = 0; i < cbSize; i++)
    {
        printf("%02X ", szBuffer[i]);
    }
    printf("\n");


done:
    LocalFree(szBuffer);
    return bResult;

}


LONG __cdecl
_tmain(
    LONG     Argc,
    LPTSTR * Argv
    )
/*++

Routine description:

    Sample program that communicates with a USB device using WinUSB

--*/
{
    DEVICE_DATA           deviceData;
    HRESULT               hr;
    USB_DEVICE_DESCRIPTOR deviceDesc;


    BOOL                  bResult;
    BOOL                  noDevice;
    ULONG                 lengthReceived;

    UNREFERENCED_PARAMETER(Argc);
    UNREFERENCED_PARAMETER(Argv);

    //
    // Find a device connected to the system that has WinUSB installed using our
    // INF
    //
    hr = OpenDevice(&deviceData, &noDevice);

    if (FAILED(hr)) {

        if (noDevice) {

            wprintf(L"Device not connected or driver not installed\n");

        } else {

            wprintf(L"Failed looking for device, HRESULT 0x%x\n", hr);
        }

        return 0;
    }

    //
    // Get device descriptor
    //
    bResult = WinUsb_GetDescriptor(deviceData.WinusbHandle,
                                   USB_DEVICE_DESCRIPTOR_TYPE,
                                   0,
                                   0,
                                   (PBYTE) &deviceDesc,
                                   sizeof(deviceDesc),
                                   &lengthReceived);

    if (FALSE == bResult || lengthReceived != sizeof(deviceDesc)) {

        wprintf(L"WinUsb_GetDescriptor: Error among LastError %d or lengthReceived %d\n",
                FALSE == bResult ? GetLastError() : 0,
                lengthReceived);
        CloseDevice(&deviceData);
        return 0;
    }

    //
    // Print a few parts of the device descriptor
    //
    wprintf(L"Device found: VID_%04X&PID_%04X; bcdUsb %04X\n",
        deviceDesc.idVendor,
        deviceDesc.idProduct,
        deviceDesc.bcdUSB);

    //PIPE_ID pipeid;
    //bResult = QueryDeviceEndpoints(deviceData.WinusbHandle, &pipeid);
    // 
    //for (int i = 0; i < 7; i++) {  
    //    WINUSB_INTERFACE_HANDLE intHandle;
    //    ZeroMemory(&intHandle, sizeof(WINUSB_INTERFACE_HANDLE));
    //    WinUsb_GetAssociatedInterface(deviceData.WinusbHandle, (UCHAR)i, &intHandle);

    //    bResult = QueryDeviceEndpoints(intHandle, &pipeid);

    //    WinUsb_Free(intHandle);
    //    if (FALSE == bResult) {

    //        wprintf(L"QueryDeviceEndpoints: Error among LastError %d\n",
    //            FALSE == bResult ? GetLastError() : 0);
    //        CloseDevice(&deviceData);
    //        return 0;
    //    }
    //}

    Controller* ctrls[4];

    ctrls[0] = new Controller(deviceData.WinusbHandle, 0);
   
    for (int i=1;i<4;i++) {
        WINUSB_INTERFACE_HANDLE intHandle;
        ZeroMemory(&intHandle, sizeof(WINUSB_INTERFACE_HANDLE));
        WinUsb_GetAssociatedInterface(deviceData.WinusbHandle, (UCHAR)i*2, &intHandle);
        ctrls[i] = new Controller(deviceData.WinusbHandle, 0);
        //ctrls[i] = std::make_shared<Controller>(deviceData.WinusbHandle, 0);
    }

    ctrls[0]->Start();
    

    
    //Sleep(10000);
    CloseDevice(&deviceData);
    return 0;
}

