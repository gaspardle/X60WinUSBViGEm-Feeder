
#include "pch.h"
#include "VigemController.h"

#include <functional>
#include <iostream>
#include <mutex>

VigemController::VigemController(int controller_id, NotifFunc cb):
    connected(false),
    m_controller_id(controller_id),    
    notification_callback(cb)
{

    client = vigem_alloc();
    target_x360 = vigem_target_x360_alloc();
}

bool VigemController::Start()
{   
    if(connected){
        return false;
    }
    
    auto ret = vigem_connect(client);
    if (!VIGEM_SUCCESS(ret) && ret != VIGEM_ERROR_BUS_ALREADY_CONNECTED) {
        printf("vigem_connect ERROR %d", ret);
        return false;
    }
       
    ret = vigem_target_add(client, target_x360);
    if (!VIGEM_SUCCESS(ret)) {
        printf("Can't add target device ERROR %d", ret);
        return false;
    }

    auto id = vigem_target_get_index(target_x360);
    printf("Starting target-id: %d\n", id);

    connected = true;

    ret = vigem_target_x360_register_notification(client, target_x360, reinterpret_cast<PFN_VIGEM_X360_NOTIFICATION>(&VigemController::notification), this);
       
    return false;
}

VOID CALLBACK VigemController::notification(
    PVIGEM_CLIENT Client,
    PVIGEM_TARGET Target,
    UCHAR LargeMotor,
    UCHAR SmallMotor,
    UCHAR LedNumber,
    LPVOID UserData
)
{    
    VigemController* inst = (VigemController*)UserData;
    inst->notification_callback(LargeMotor, SmallMotor, LedNumber);
}
bool VigemController::Stop()
{
    if(connected){
        vigem_target_x360_unregister_notification(target_x360);
        vigem_target_remove(client, target_x360);
    }
    connected = false;
    return true;
}

VigemController::~VigemController()
{    
    if (connected) {
        Stop();
    }    
    vigem_target_free(target_x360);

    vigem_free(client);
}

bool VigemController::Update(x360_report_t& nativereport)
{
    if (!connected) {
        return false;    
    }

    XUSB_REPORT report;
    XUSB_REPORT_INIT(&report);
    report.bLeftTrigger = nativereport.z;
    report.bRightTrigger = nativereport.rz;
    report.wButtons = _byteswap_ushort(nativereport.buttons);
    report.sThumbLX = nativereport.x;
    report.sThumbLY = nativereport.y;
    report.sThumbRX = nativereport.rx;
    report.sThumbRY = nativereport.ry;

    auto ret = vigem_target_x360_update(client, target_x360, report);
    if (VIGEM_SUCCESS(ret)) {
        return true;
    }
    return false;
}


