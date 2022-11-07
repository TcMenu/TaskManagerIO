
#include <Arduino.h>
#include <testing/SimpleTest.h>
#include <Wire.h>

using namespace SimpleTest;

void setup() {
    Serial.begin(115200);
    while (!Serial);
    startTesting();
}

DEFAULT_TEST_RUNLOOP
