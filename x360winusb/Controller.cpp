//#include <winusb.h>

#include "pch.h"
#include <iostream>
#include <thread>

//#include "device.h"


Controller::Controller(WINUSB_INTERFACE_HANDLE hDev, int controller_id):
m_interface(hDev),
m_controller_id(controller_id)
{
    int bResult;
    m_controller_id = controller_id;
    bResult = QueryDeviceEndpoints(hDev, &m_endpoints);
}

Controller::~Controller()
{
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
    //WinUsb_SetPipePolicy(hDeviceHandle, pID, PIPE_TRANSFER_TIMEOUT, sizeof(ULONG), 500);

    BOOL bResult = FALSE;
    //BOOL oResult = FALSE;
    UCHAR* szBuffer = (UCHAR*)LocalAlloc(LPTR, sizeof(UCHAR) * cbSize);

    ULONG cbRead = 0;


    OVERLAPPED overlapped;
    ZeroMemory(&overlapped, sizeof(overlapped));
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);


    bResult = WinUsb_ReadPipe(hDeviceHandle, pID, szBuffer, cbSize, &cbRead, &overlapped);       
    if (!bResult && GetLastError() != ERROR_IO_PENDING)
    {
        goto done;
    }

    BindIoCompletionCallback(hDeviceHandle, callbOverlappedCompletionRoutine, 0);
   /* oResult = WinUsb_GetOverlappedResult(hDeviceHandle, &overlapped, &cbRead, FALSE);
    auto x= GetLastError();
    if (oResult == FALSE && (x == ERROR_IO_PENDING || x == ERROR_IO_INCOMPLETE)) {
       // printf("not ready \n ");
        goto done;
    }

    printf("id %d: len: %d : ", m_controller_id, cbRead);
    for (ULONG i = 0; i < cbRead; i++)
    {
        printf("%02X ", szBuffer[i]);
    }
    printf("\n");
    ParseMessage(szBuffer, cbRead);
    */
done:
    LocalFree(szBuffer);
    return bResult;

}
//

void Controller::set_led_real(uint8_t status)
{
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
            set_led_real(2 + m_controller_id % 4);
            //set_active(true);
        }
        else if (data[1] == 0x40)
        {
            printf("Connection status: headset connected\n");
        }
        else if (data[1] == 0xc0)
        {
            printf("Connection status: controller and headset connected\n");
            set_led_real(2 + m_controller_id % 4);
        }
        else
        {
            printf("Connection status: unknown\n");
        }
    }
    else if (len == 29)
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
            m_battery_status = data[17];
            printf("Serial: %s\n", "xxx");
            std::cout << "Battery Status: \n" << m_battery_status;
        }
        else if (data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0x13)
        { // Battery status
            m_battery_status = data[4];
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
    return true;
}
