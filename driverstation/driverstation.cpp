#include "driverstation.h"
#include "wpi/mutex.h"
#include "wpi/static_circular_buffer.h"
#include <queue>
#include <atomic>

#include "dsevents.h"
#include "robotcomms.h"

ds::DsEvents* DsEvents;
ds::RobotComms* RobotComms;

extern "C" {
DS_Status DS_Initialize(const DS_EventCallbacks* EventCallbacks) {
    std::unique_ptr<ds::DsEvents> LocalDsEvents;
    std::unique_ptr<ds::RobotComms> LocalRobotComms;
    if (DsEvents) {
        return DS_ALREADY_INITIALIZED;
    }
    if (!EventCallbacks || !EventCallbacks->SetEvent) {
        return DS_INVALID_PARAMETER;
    }
    LocalDsEvents.reset(new(std::nothrow) ds::DsEvents{EventCallbacks});
    if (!LocalDsEvents) {
        return DS_OUT_OF_MEMORY;
    }
    DS_Status Status = DS_OUT_OF_MEMORY;
    LocalRobotComms.reset(new(std::nothrow) ds::RobotComms{LocalDsEvents.get(), &Status});
    if (Status != DS_STATUS_SUCCESS) {
        return DS_OUT_OF_MEMORY;
    }


    RobotComms = LocalRobotComms.release();
    DsEvents = LocalDsEvents.release();
    return DS_STATUS_SUCCESS;
}

void DS_Cleanup(void) {
    if (DsEvents) {
        delete DsEvents;
        DsEvents = nullptr;
    }
}

void SetTeam(const char* Name) {
    (void)Name;
}

uint32_t DS_ReadEvents(DS_Event* Events, uint32_t EventCount, uint32_t* RemainingEvents) {
    return DsEvents->ReadEvents(Events, EventCount, RemainingEvents);
}

void DS_FreeEvents(DS_Event* Events, uint32_t EventCount) {
    (void)Events;
    (void)EventCount;
}
}
