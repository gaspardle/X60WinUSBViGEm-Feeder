#pragma once
#include <winusb.h>
#include <cstdint>
#include "VigemController.h"

class Controller
{
private:
    int  m_controller_id;

    WINUSB_INTERFACE_HANDLE  m_interface;
    int  m_battery_status;
    BOOL Controller::ReadFromBulkEndpoint(WINUSB_INTERFACE_HANDLE hDeviceHandle, UCHAR pID, ULONG cbSize);
    PIPE_ID m_endpoints;
    VigemController* emulated_controller;

public:
    Controller(WINUSB_INTERFACE_HANDLE hDev, int controller_id);
    virtual ~Controller();

    bool Start();
    BOOL ReadAndParse();
    bool ParseMessage(const uint8_t* data, int len);
    void set_led_real(uint8_t status);
    UINT get_controller_id();
    //void set_rumble_real(uint8_t left, uint8_t right);
    //void set_led_real(uint8_t status);
    //uint8_t get_battery_status() const;

};

