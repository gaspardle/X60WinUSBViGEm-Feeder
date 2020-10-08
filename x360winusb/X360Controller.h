#pragma once

#include <Windows.h>
#include <winusb.h>
#include <cstdint>

#include "device.h"
#include "VigemController.h"

class X360Controller
{
private:
    int m_controller_id;
    WINUSB_INTERFACE_HANDLE  m_interface;    
    BOOL X360Controller::ReadFromBulkEndpoint(WINUSB_INTERFACE_HANDLE hDeviceHandle, UCHAR pID);
    bool ParseMessage(const uint8_t* data, int len);
    PIPE_ID m_endpoints;
    VigemController* emulated_controller;

public:
    X360Controller(WINUSB_INTERFACE_HANDLE hDev, int controller_id);
    virtual ~X360Controller();

    BOOL ReadAndParse();
   
    void SetLED(uint8_t status);
    void SetRumble(uint8_t leftMotorSpeed, uint8_t rightMotorSpeed);
  
};


