//#include <winusb.h>

#include "pch.h"
#include <iostream>
#include <thread>
#include <ViGEm/Client.h>
#include "Xbox.h"
//#include "device.h"

Controller::Controller(WINUSB_INTERFACE_HANDLE hDev, int controller_id):
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
        [&](uint8_t large, uint8_t small, uint8_t led) {         
            set_led(led + 2);
            set_rumble(large, small);
        }
     );
}

Controller::~Controller()
{
    emulated_controller->Stop();
    delete emulated_controller;
    WinUsb_Free(m_interface);
}

bool Controller::Start(){
    while (true) {
        ReadFromBulkEndpoint(m_interface, m_endpoints.PipeInId, 32);
    }
}

UINT Controller::get_controller_id() {
    return m_controller_id;
}

VOID WINAPI callbOverlappedCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransferred, LPOVERLAPPED pOverlapped) {
 auto c = pOverlapped;
 c=c;
    printf("callb %d  %d \n ", dwErrorCode, dwNumberOfBytesTransferred);
}

BOOL Controller::ReadFromBulkEndpoint(WINUSB_INTERFACE_HANDLE hDeviceHandle, UCHAR pID, ULONG cbSize)
{
    if (hDeviceHandle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }
    //

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
//
void Controller::set_rumble(uint8_t left, uint8_t right)
{
    BOOL bResult = TRUE;
    ULONG cbTransferred = 0;
    uint8_t rumblecmd[] = { 0x00, 0x01, 0x0f, 0xc0, 0x00, left, right, 0x00, 0x00, 0x00, 0x00, 0x00 };
    bResult = WinUsb_WritePipe(m_interface, m_endpoints.PipeOutId, rumblecmd, sizeof(rumblecmd), &cbTransferred, 0);    
}


void Controller::set_led(uint8_t status)
{
    printf("set_led: %d\n", status);
    BOOL bResult = TRUE;
    ULONG cbTransferred = 0;

    uint8_t ledcmd[] = { 0x00, 0x00, 0x08, static_cast<uint8_t>(0x40 + (status % 0x0e)), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    bResult = WinUsb_WritePipe(m_interface, m_endpoints.PipeOutId, ledcmd, sizeof(ledcmd), &cbTransferred, 0);
}

BOOL Controller::ReadAndParse() {
    return ReadFromBulkEndpoint(m_interface, m_endpoints.PipeInId, 32);
}
bool Controller::ParseMessage(const uint8_t* data, int len)
{
    if (len == 2 && data[0] == 0x08)
    { 
        // Connection status
        if (data[1] == 0x00)
        {
            printf("connection status: not connected\n");
            emulated_controller->Stop();
        }
        else if (data[1] == 0x80)
        {
            printf("connection status: controller connected\n");            
            emulated_controller->Start();
           // set_led_real(2 + m_controller_id % 4);          
        }
        else if (data[1] == 0x40)
        {
            printf("Connection status: headset connected\n");
        }
        else if (data[1] == 0xc0)
        {
            printf("Connection status: controller and headset connected\n");            
            emulated_controller->Start();
            //set_led_real(2 + m_controller_id % 4);
        }
        else
        {
            printf("Connection status: unknown\n");
        }
    }
    else if (data[1] == 0x0f && data[2] == 0x00 && data[3] == 0xf0)
    { 
        // Initial
        /*m_serial = (boost::format("%2x:%2x:%2x:%2x:%2x:%2x:%2x")
            % int(data[7])
            % int(data[8])
            % int(data[9])
            % int(data[10])
            % int(data[11])
            % int(data[12])
            % int(data[13])).str();
         printf("Serial: %s\n", "xxx");
            */
        m_battery_status = data[17];
       
        printf("Battery status0f: %d\n", m_battery_status);
    }
    else if (data[1] == 0x00 && data[3] == 0x13)
    { 
        m_battery_status = data[4];
        printf("Battery status00: %d\n", m_battery_status);
    }
    else if (data[1] == 0x00 && data[2] == 0x00 && data[3] == 0xf0)
    {
        // not useful
    }
    else if (data[1] == 0x01 /*&& data[2] == 0x00*/ && data[3] == 0xf0)
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
       /* printf("update \n");
        printf("dup : %d \n", (buttons & (uint16_t)Buttons::DPadUp) == (uint16_t)Buttons::DPadUp);
        printf("back : %d \n", (buttons & (uint16_t)Buttons::Back) == (uint16_t)Buttons::Back);
        printf("x : %d \n", (buttons & (uint16_t)Buttons::X) == (uint16_t)Buttons::X);

      
        printf("trigger: %d %d \n", report.z, report.rz);
        printf("stick: x %d y %d \n", report.x, report.y);
        printf("stick: rx %d ry %d \n", report.rx, report.ry);*/
    }   
    else
    {
        printf("unknown command \n");
    }

   
    return true;
}
