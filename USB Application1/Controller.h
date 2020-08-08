#pragma once
#include <winusb.h>
#include <cstdint>

class Controller
{
private:
    int  m_controller_id;

    WINUSB_INTERFACE_HANDLE  m_interface;
    int  m_battery_status;
    BOOL Controller::ReadFromBulkEndpoint(WINUSB_INTERFACE_HANDLE hDeviceHandle, UCHAR pID, ULONG cbSize);
    PIPE_ID m_endpoints;

public:
    Controller(WINUSB_INTERFACE_HANDLE hDev, int controller_id);
    virtual ~Controller();

    bool Start();
    bool ParseMessage(const uint8_t* data, int len);
    void Controller::set_led_real(uint8_t status);
    //void set_rumble_real(uint8_t left, uint8_t right);
    //void set_led_real(uint8_t status);
    //uint8_t get_battery_status() const;

};

