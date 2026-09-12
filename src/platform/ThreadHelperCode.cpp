
#include "../TaskPlatformDeps.h"

#ifdef PICO_NEEDS_PROTECTOR
//
// On PicoSDK we need allocate and prepare the critical section after the platform has been initialized. If we don't
// do this, we can get a crash when the first task is created. This pretty much as to be created with new, but it is
// of absolutely minimal size. It is the class/heap overhead plus the size of the critical section itself.
//
volatile TmCriticalSectionPico* globalPicoCs = nullptr;
inline void tmInitAtomics() {
    if (globalPicoCs == nullptr) {
        globalPicoCs = new TmCriticalSectionPico();
    }
}
#else
inline void tmInitAtomics() {
    // nothing to do on most boards
}
#endif

