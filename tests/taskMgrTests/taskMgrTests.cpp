
#include <IoAbstraction.h>
#include <testing/SimpleTest.h>

using namespace SimpleTest;

IOLOG_MBED_PORT_IF_NEEDED(USBTX, USBRX)

void setup() {
    IOLOG_START_SERIAL
    startTesting();
}

DEFAULT_TEST_RUNLOOP
