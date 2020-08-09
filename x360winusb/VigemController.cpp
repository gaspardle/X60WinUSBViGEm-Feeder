#include "pch.h"

#include "VigemController.h"
#include <functional>
#include <iostream>
#include <mutex>

VigemController::VigemController(int controller_id):
    connected(false)
{
    controller_id;
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
        return false;
    }
       
    ret = vigem_target_add(client, target_x360);
    if (!VIGEM_SUCCESS(ret)) {
        return false;
    }

    connected = true;

    ret = vigem_target_x360_register_notification(client, target_x360, reinterpret_cast<PFN_VIGEM_X360_NOTIFICATION>(&VigemController::notification), nullptr);
    //ret = vigem_target_x360_register_notification(client, target_x360, &VigemController::notification, nullptr);
    
    return false;
}

static std::mutex m;
VOID CALLBACK VigemController::notification(
    PVIGEM_CLIENT Client,
    PVIGEM_TARGET Target,
    UCHAR LargeMotor,
    UCHAR SmallMotor,
    UCHAR LedNumber,
    LPVOID UserData
)
{
    //m.lock();
    printf("notif led:%d\n", LedNumber);
    /*static int count = 1;

    std::cout.width(3);
    std::cout << count++ << " ";
    std::cout.width(3);
    std::cout << (int)LargeMotor << " ";
    std::cout.width(3);
    std::cout << (int)SmallMotor << std::endl;
    */
    //m.unlock();
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
    report.sThumbRX = nativereport.ry;
    report.sThumbRY = nativereport.ry;

    auto ret = vigem_target_x360_update(client, target_x360, report);
    if (VIGEM_SUCCESS(ret)) {
        return true;
    }
    return false;
}


