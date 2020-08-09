#include "pch.h"

#include <stdio.h>
#include <cstdint>
#include "main.h"
#include <iostream>
#include <thread>


void doSomething(Controller* c) {
    while (true) {
        //printf("do: %d\n", c->get_controller_id());
        c->ReadAndParse();
        //Sleep(3000);
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

    //
    // Print a few parts of the device descriptor
    //
    wprintf(L"Device found: VID_%04X&PID_%04X; bcdUsb %04X\n",
        deviceDesc.idVendor,
        deviceDesc.idProduct,
        deviceDesc.bcdUSB);

    Controller* ctrls[4];

    ctrls[0] = new Controller(deviceData.WinusbHandle, 0);
   
    for (int i=1;i<4;i++) {
        WINUSB_INTERFACE_HANDLE intHandle;
        ZeroMemory(&intHandle, sizeof(WINUSB_INTERFACE_HANDLE));
        WinUsb_GetAssociatedInterface(deviceData.WinusbHandle, (UCHAR)i*2, &intHandle);
        ctrls[i] = new Controller(intHandle, i);
        //ctrls[i] = std::make_shared<Controller>(deviceData.WinusbHandle, 0);
    }

    std::thread t1(doSomething, ctrls[0]);
    std::thread t2(doSomething, ctrls[1]);
    std::thread t3(doSomething, ctrls[2]);
    std::thread t4(doSomething, ctrls[3]);

    //t1.join();
    //t2.join();

    while (!(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        printf("doing things");
       // ret = vigem_target_x360_update(client, x360, report);
       // report.bLeftTrigger++;
        Sleep(1000);
    }
   
    DWORD tresult = ::TerminateThread(t1.native_handle(), 1);
    tresult = ::TerminateThread(t2.native_handle(), 1);
    tresult = ::TerminateThread(t3.native_handle(), 1);
    tresult = ::TerminateThread(t4.native_handle(), 1);

    t1.std::thread::~thread();
    t2.std::thread::~thread();
    t3.std::thread::~thread();
    t4.std::thread::~thread();
    //Sleep(10000)
    
    /*delete ctrls[1];
    delete ctrls[2];
    delete ctrls[3];
    delete ctrls[0];*/

    CloseDevice(&deviceData);
    return 0;
}

