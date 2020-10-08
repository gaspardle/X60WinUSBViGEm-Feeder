#include <Windows.h>

#include <winusb.h>
//#include <usb.h>
#include <cstdio>
#include <cstdint>
#include <thread>
#include <ViGEm\Client.h>
#include "X360Controller.h"
#include "device.h"

bool processInputs = true;

void doSomething(X360Controller* c) {
    while (processInputs) {
        c->ReadAndParse();
    }
}

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType)
    {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
        std::printf("Exiting...\n\n");
        processInputs = false;
        Sleep(10000);
        return TRUE;

    default:
        return FALSE;
    }
}

int wmain(int argc, wchar_t* argv[])
{
    DEVICE_DATA           deviceData;
    HRESULT               hr;
    USB_DEVICE_DESCRIPTOR deviceDesc;


    BOOL                  bResult;
    BOOL                  noDevice;
    ULONG                 lengthReceived;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    //
    // Find a device connected to the system that has WinUSB installed using our
    // INF
    //
    hr = OpenDevice(&deviceData, &noDevice);

    if (FAILED(hr)) {

        if (noDevice) {
            std::printf("Receiver not connected or driver not installed\n");
        }
        else {
            std::printf("Failed looking for device, HRESULT 0x%x\n", hr);
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
        (PBYTE)&deviceDesc,
        sizeof(deviceDesc),
        &lengthReceived);

    if (FALSE == bResult || lengthReceived != sizeof(deviceDesc)) 
    {
        std::printf("WinUsb_GetDescriptor: Error among LastError %d or lengthReceived %d\n",
            FALSE == bResult ? GetLastError() : 0,
            lengthReceived);
        CloseDevice(&deviceData);
        return 0;
    }

    //
    // Print a few parts of the device descriptor
    //
    std::printf("Device found: VID_%04X&PID_%04X; bcdUsb %04X\n",
        deviceDesc.idVendor,
        deviceDesc.idProduct,
        deviceDesc.bcdUSB);

    X360Controller* ctrls[4];
    ctrls[0] = new X360Controller(deviceData.WinusbHandle, 0);

    for (int i = 1; i < 4; i++) {
        WINUSB_INTERFACE_HANDLE intHandle;
        ZeroMemory(&intHandle, sizeof(WINUSB_INTERFACE_HANDLE));
        WinUsb_GetAssociatedInterface(deviceData.WinusbHandle, (UCHAR)(i * 2) - 1, &intHandle);

        ctrls[i] = new X360Controller(intHandle, i);
    }

    std::thread t1(doSomething, ctrls[0]);
    std::thread t2(doSomething, ctrls[1]);
    std::thread t3(doSomething, ctrls[2]);
    std::thread t4(doSomething, ctrls[3]);

    SetConsoleCtrlHandler(CtrlHandler, TRUE);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    delete ctrls[0];
    delete ctrls[1];
    delete ctrls[2];
    delete ctrls[3];

    CloseDevice(&deviceData);
    return 0;
}

