#pragma once
#include "Xbox.h"
#include  <functional>

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
    static VOID CALLBACK notification(PVIGEM_CLIENT Client, PVIGEM_TARGET Target, UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber, LPVOID UserData);
    bool Stop();
    bool Update(x360_report_t& report);

};

