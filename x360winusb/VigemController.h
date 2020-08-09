#pragma once
#include "Xbox.h"

class VigemController
{
private:
    PVIGEM_CLIENT client;
    PVIGEM_TARGET target_x360;
    bool connected;

public:
    VigemController(int controller_id);
    virtual ~VigemController();

    bool Start();
    static VOID CALLBACK notification(PVIGEM_CLIENT Client, PVIGEM_TARGET Target, UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber, LPVOID UserData);
    bool Stop();
    bool Update(x360_report_t& report);

};

