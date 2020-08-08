#include "pch.h"

#include <stdio.h>
#include <cstdint>
#include "main.h"
#include <iostream>


struct PIPE_ID
{
    UCHAR  PipeInId;
    UCHAR  PipeOutId;
};

BOOL QueryDeviceEndpoints(WINUSB_INTERFACE_HANDLE hDeviceHandle, PIPE_ID* pipeid)
{
    if (hDeviceHandle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    BOOL bResult = TRUE;

    USB_INTERFACE_DESCRIPTOR InterfaceDescriptor;
    ZeroMemory(&InterfaceDescriptor, sizeof(USB_INTERFACE_DESCRIPTOR));

    WINUSB_PIPE_INFORMATION  Pipe;
    ZeroMemory(&Pipe, sizeof(WINUSB_PIPE_INFORMATION));


    bResult = WinUsb_QueryInterfaceSettings(hDeviceHandle, 0, &InterfaceDescriptor);

    if (bResult)
    {
        for (int index = 0; index < InterfaceDescriptor.bNumEndpoints; index++)
        {
            bResult = WinUsb_QueryPipe(hDeviceHandle, 0, (UCHAR)index, &Pipe);

            if (bResult)
            {
                if (Pipe.PipeType == UsbdPipeTypeControl)
                {
                    printf("Endpoint index: %d Pipe type: %d Control, Pipe ID: %x.\n", index, Pipe.PipeType, Pipe.PipeId);
                }
                if (Pipe.PipeType == UsbdPipeTypeIsochronous)
                {
                    printf("Endpoint index: %d Pipe type: %d Isochronous, Pipe ID: %x.\n", index, Pipe.PipeType, Pipe.PipeId);
                }
                if (Pipe.PipeType == UsbdPipeTypeBulk)
                {
                    if (USB_ENDPOINT_DIRECTION_IN(Pipe.PipeId))
                    {
                        printf("Endpoint index: %d Pipe type: %d Bulk, Pipe ID: %x.\n", index, Pipe.PipeType, Pipe.PipeId);
                        pipeid->PipeInId = Pipe.PipeId;
                    }
                    if (USB_ENDPOINT_DIRECTION_OUT(Pipe.PipeId))
                    {
                        printf("Endpoint index: %d Pipe type: %d Bulk, Pipe ID: %x.\n", index, Pipe.PipeType, Pipe.PipeId);
                        pipeid->PipeOutId = Pipe.PipeId;
                    }

                }
                if (Pipe.PipeType == UsbdPipeTypeInterrupt)
                {
                    if (USB_ENDPOINT_DIRECTION_IN(Pipe.PipeId))
                    {
                        printf("Endpoint IN index: %d Pipe type: %d Interrupt, Pipe ID: %x.\n", index, Pipe.PipeType, Pipe.PipeId);
                        pipeid->PipeInId = Pipe.PipeId;
                    }
                    if (USB_ENDPOINT_DIRECTION_OUT(Pipe.PipeId))
                    {
                        printf("Endpoint OUT index: %d Pipe type: %d Interrupt, Pipe ID: %x.\n", index, Pipe.PipeType, Pipe.PipeId);
                        pipeid->PipeOutId = Pipe.PipeId;
                    }                   
                }
            }
            else
            {
                continue;
            }
        }
    }

    return bResult;
}

BOOL ReadFromBulkEndpoint(WINUSB_INTERFACE_HANDLE hDeviceHandle, UCHAR pID, ULONG cbSize)
{
    if (hDeviceHandle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    BOOL bResult = TRUE;
    UCHAR* szBuffer = (UCHAR*)LocalAlloc(LPTR, sizeof(UCHAR) * cbSize);
   
    ULONG cbRead = 0;
    
    bResult = WinUsb_ReadPipe(hDeviceHandle, pID, szBuffer, cbSize, &cbRead, 0);
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
    Parse(szBuffer, cbRead);

done:
    LocalFree(szBuffer);
    return bResult;

}

void Parse(const UCHAR* data, int len) {
    if (len == 2 && data[0] == 0x08)
    { // Connection Status Message
        if (data[1] == 0x00)
        {
            printf("connection status: nothing\n");

            // reset the controller into neutral position on disconnect
            //msg_out->clear();
            //set_active(false);

            //return true;
        }
        else if (data[1] == 0x80)
        {
            printf("connection status: controller connected\n");
            //set_led_real(get_led());
            //set_active(true);
        }
        else if (data[1] == 0x40)
        {
            printf("Connection status: headset connected\n");
        }
        else if (data[1] == 0xc0)
        {
            printf("Connection status: controller and headset connected\n");
            //set_led_real(get_led());
        }
        else
        {
            printf("Connection status: unknown\n");
        }
    }
    else if(len==29) 
    {
        //set_active(true);
        if (data[0] == 0x00 && data[1] == 0x0f && data[2] == 0x00 && data[3] == 0xf0)
        { // Initial Announc Message
            /*m_serial = (boost::format("%2x:%2x:%2x:%2x:%2x:%2x:%2x")
                % int(data[7])
                % int(data[8])
                % int(data[9])
                % int(data[10])
                % int(data[11])
                % int(data[12])
                % int(data[13])).str();*/
            int m_battery_status = data[17];
            printf("Serial: %s\n", "xxx");
            std::cout << "Battery Status: \n" << m_battery_status;
        }
        else if (data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0x13)
        { // Battery status
            //m_battery_status = data[4];
            printf("battery status: %d\n", data[4]);
        }
        else if (data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0xf0)
        {
            // 0x00 0x00 0x00 0xf0 0x00 ... is send after each button
            // press, doesn't seem to contain any information
        }
        else if (data[0] == 0x00 && data[1] == 0x01 && data[2] == 0x00 && data[3] == 0xf0 && data[4] == 0x00 && data[5] == 0x13)
        {
            printf("event \n");
        }
        else
        {
            printf("unknown: \n");
        }

    }
    else {
        printf("unknown: \n");
    }
   

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

    PIPE_ID pipeid;
    bResult = QueryDeviceEndpoints(deviceData.WinusbHandle, &pipeid);
  


   
    for (int i = 0; i < 7; i++) {  
        WINUSB_INTERFACE_HANDLE intHandle;
        ZeroMemory(&intHandle, sizeof(WINUSB_INTERFACE_HANDLE));
        WinUsb_GetAssociatedInterface(deviceData.WinusbHandle, (UCHAR)i, &intHandle);

        bResult = QueryDeviceEndpoints(intHandle, &pipeid);

        WinUsb_Free(intHandle);
        if (FALSE == bResult) {

            wprintf(L"QueryDeviceEndpoints: Error among LastError %d\n",
                FALSE == bResult ? GetLastError() : 0);
            CloseDevice(&deviceData);
            return 0;
        }
    }

    while(true){
        ReadFromBulkEndpoint(deviceData.WinusbHandle, 0x81 , 32 );
    }
    //
    // Print a few parts of the device descriptor
    //
    wprintf(L"Device found: VID_%04X&PID_%04X; bcdUsb %04X\n",
            deviceDesc.idVendor,
            deviceDesc.idProduct,
            deviceDesc.bcdUSB);
    //Sleep(10000);
    CloseDevice(&deviceData);
    return 0;
}

