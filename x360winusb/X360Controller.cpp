//#include <winusb.h>
#include <iostream>
#include <thread>

#include "X360Controller.h"
#include "XboxControllerValues.h"

X360Controller::X360Controller(WINUSB_INTERFACE_HANDLE hDev, int controller_id) :
    m_interface(hDev),
    m_controller_id(controller_id)
{
    int bResult;
    bResult = QueryDeviceEndpoints(hDev, &m_endpoints);

    ULONG timeout = 60;
    ULONG raw = 1;
    WinUsb_SetPipePolicy(hDev, m_endpoints.PipeInId, PIPE_TRANSFER_TIMEOUT, sizeof(ULONG), &timeout);
    WinUsb_SetPipePolicy(hDev, m_endpoints.PipeInId, RAW_IO, sizeof(ULONG), &raw);

    emulated_controller = new VigemController(controller_id,
        [&](uint8_t largemotor, uint8_t smallmotor, uint8_t led) {
            printf("set lm: %d sm: %d led: %d\n", largemotor, smallmotor, led);
            SetRumble(largemotor, smallmotor);
            SetLED(led + 2);
        }
    );
}

X360Controller::~X360Controller()
{
    emulated_controller->Stop();
    delete emulated_controller;
    WinUsb_Free(m_interface);
}

BOOL X360Controller::ReadFromBulkEndpoint(WINUSB_INTERFACE_HANDLE hDeviceHandle, UCHAR pID)
{
    if (hDeviceHandle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }
    //
    ULONG cbSize = 32;
    BOOL bResult = FALSE;
    //BOOL oResult = FALSE;
    UCHAR* szBuffer = (UCHAR*)LocalAlloc(LPTR, sizeof(UCHAR) * cbSize);
    ULONG cbRead = 0;

    //OVERLAPPED overlapped;
    //ZeroMemory(&overlapped, sizeof(overlapped));
    //overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);


    bResult = WinUsb_ReadPipe(hDeviceHandle, pID, szBuffer, cbSize, &cbRead, 0 /*&overlapped*/);
    if (!bResult) {
        auto last_error = GetLastError();
        if (last_error == ERROR_SEM_TIMEOUT) {
            //printf("timeout id %d\n", m_controller_id);
            goto done;
        }
        if (last_error != ERROR_IO_PENDING) //ERROR_SEM_TIMEOUT
        {
            goto done;
        }
    }

    // BindIoCompletionCallback(hDeviceHandle, callbOverlappedCompletionRoutine, 0);
    /* oResult = WinUsb_GetOverlappedResult(hDeviceHandle, &overlapped, &cbRead, FALSE);
     auto x= GetLastError();
     if (oResult == FALSE && (x == ERROR_IO_INCOMPLETE)) {
        // printf("not ready \n ");
         goto done;
     }
     */
     /*  printf("id %d: len: %d : ", m_controller_id, cbRead);
       for (ULONG i = 0; i < cbRead; i++)
       {
           printf("%02X ", szBuffer[i]);
       }
       printf("\n");*/
    ParseMessage(szBuffer, cbRead);

done:
    LocalFree(szBuffer);
    return bResult;

}

void X360Controller::SetRumble(uint8_t leftMotorSpeed, uint8_t rightMotorSpeed)
{
    BOOL bResult = FALSE;
    ULONG cbTransferred = 0;

    uint8_t cmd[] = { 0x00, 0x01, 0x0f, 0xc0, 0x00, leftMotorSpeed, rightMotorSpeed, 0x00, 0x00, 0x00, 0x00, 0x00 };
    bResult = WinUsb_WritePipe(m_interface, m_endpoints.PipeOutId, cmd, sizeof(cmd), &cbTransferred, 0);
}


void X360Controller::SetLED(uint8_t status)
{
    BOOL bResult = FALSE;
    ULONG cbTransferred = 0;
 
    uint8_t cmd[] = { 0x00, 0x00, 0x08, static_cast<uint8_t>(0x40 + (status % 0x0e)), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    bResult = WinUsb_WritePipe(m_interface, m_endpoints.PipeOutId, cmd, sizeof(cmd), &cbTransferred, 0);
}

BOOL X360Controller::ReadAndParse() {
    return ReadFromBulkEndpoint(m_interface, m_endpoints.PipeInId);
}

bool X360Controller::ParseMessage(const uint8_t* data, int len)
{

    if (data[0] == 0x08)
    {
        // Connection status
        if (data[1] == 0x00)
        {
            printf("connection status: not connected\n");
            emulated_controller->Stop();
        }
        else if (data[1] == 0x40)
        {
            printf("Connection status: headset connected\n");
        }
        else if (data[1] == 0x80)
        {
            printf("connection status: controller connected\n");
            emulated_controller->Start();
        }
        else if (data[1] == 0xc0)
        {
            printf("Connection status: controller and headset connected\n");
            emulated_controller->Start();
        }
        else
        {
            printf("Connection status: unknown\n");
        }
    }
    else if (data[1] == 0x0f && data[2] == 0x00 && data[3] == 0xf0)
    {
        printf("Battery status: %d\n", data[17]);
    }
    else if (data[1] == 0x00 && data[3] == 0x13)
    {
        printf("Battery status: %d\n", data[4]);
    }
    else if (data[1] == 0x00 && data[2] == 0x00 && data[3] == 0xf0)
    {
        // not useful
        //printf("notuseful\n");
    }
    else if (data[1] == 0x01 && data[3] == 0xf0)
    {
        emulated_controller->Start();

        x360_report_t report;
        uint16_t buttons = ((uint16_t)data[6] << 8) | data[7];

        report.buttons = buttons;
        report.z = data[8];
        report.rz = data[9];
        report.x = (data[11] << 8) | data[10];
        report.y = (data[13] << 8) | data[12];
        report.rx = (data[15] << 8) | data[14];
        report.ry = (data[17] << 8) | data[16];

        emulated_controller->Update(report);
        //update
        if ((buttons & (uint16_t)Buttons::XboxGuide) == (uint16_t)Buttons::XboxGuide) {
            printf("g\n");
        }
        /*printf("update \n");
        printf("dup : %d \n", (buttons & (uint16_t)Buttons::DPadUp) == (uint16_t)Buttons::DPadUp);
        printf("back : %d \n", (buttons & (uint16_t)Buttons::Back) == (uint16_t)Buttons::Back);
        printf("x : %d \n", (buttons & (uint16_t)Buttons::X) == (uint16_t)Buttons::X);


        printf("trigger: %d %d \n", report.z, report.rz);
        printf("stick: x %d y %d \n", report.x, report.y);
        printf("stick: rx %d ry %d \n", report.rx, report.ry);*/
    }

    return true;
}
