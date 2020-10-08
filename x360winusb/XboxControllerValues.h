#pragma once

#include <cstdint>

struct x360_report_t
{
    uint16_t buttons;

    //trigger
    uint8_t z;
    uint8_t rz;
    //sticks
    int16_t x;
    int16_t y;
    int16_t rx;
    int16_t ry;
};


enum class Buttons : uint16_t {
    RT = 0x8000,
    LT = 0x4000,
    Back = 0x2000,
    Start = 0x1000,
    DPadRight = 0x0800,
    DPadLeft = 0x0400,
    DPadDown = 0x0200,
    DPadUp = 0x0100,
    Y = 0x0080,
    X = 0x0040,
    B = 0x0020,
    A = 0x0010,
    btnReserved1 = 0x0008,
    XboxGuide = 0x0004,
    RB = 0x0002,
    LB = 0x0001
};


enum LEDValues {
    ledOff = 0x00,
    ledBlinkingAll = 0x01,
    ledFlashOn1 = 0x02,
    ledFlashOn2 = 0x03,
    ledFlashOn3 = 0x04,
    ledFlashOn4 = 0x05,
    ledOn1 = 0x06,
    ledOn2 = 0x07,
    ledOn3 = 0x08,
    ledOn4 = 0x09,
    ledRotating = 0x0a,
    ledBlinking = 0x0b, // Blinking of previously enabled LED (e.g. from 0x01-0x09)
    ledBlinkingSlow = 0x0c, // As above
    ledAlternating = 0x0d  // 1+4, 2+3, then back to previous after a short time
};