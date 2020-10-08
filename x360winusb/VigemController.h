#pragma once

//#include <Windows.h>
#include <ViGEm\Client.h>
#include <functional>
#include <wtypes.h>

#include "XboxControllerValues.h"

typedef std::function<void(uint8_t, uint8_t, uint8_t)> NotifFunc;

class VigemController
{
private:
    PVIGEM_CLIENT client;
    PVIGEM_TARGET target_x360;
    bool connected;
    int m_controller_id;
    NotifFunc notification_callback;

public:
    VigemController(int controller_id, NotifFunc cb);
    virtual ~VigemController();
    bool Start();   
    bool Stop();
    bool Update(x360_report_t &report);
    //static VOID CALLBACK OnNotification(PVIGEM_CLIENT Client, PVIGEM_TARGET Target, UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber, LPVOID UserData);
    static void OnNotification(PVIGEM_CLIENT Client, PVIGEM_TARGET Target, UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber, LPVOID UserData);

};

