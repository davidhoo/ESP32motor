#pragma once

#include <Arduino.h>
#include "../controllers/MotorModbusController.h"

class ModbusTest {
public:
    static void runAllTests();
    static void testCRCCalculation();
    static void testFrameFormat();
    static void testMotorCommunication();
    static void testParameterReading();
    static void testParameterWriting();
    static void testErrorHandling();
    
private:
    static void printTestHeader(const String& testName);
    static void printTestResult(const String& testName, bool passed);
    static void printFrame(const uint8_t* frame, uint16_t length, const String& description);
};