#pragma once

#include "stdint.h"

#ifdef _WIN32
#define DS_API __cdecl
#else
#define DS_API
#endif

typedef void (DS_API * DS_SetEventCallback)(void* Context);
typedef void (DS_API * DS_ResetEventCallback)(void* Context);

typedef struct DS_EventCallbacks {
    void* Context;
    DS_SetEventCallback SetEvent;
} DS_EventCallbacks;

typedef enum DS_EventType {
    DS_EventType_Connected,
    DS_EventType_Disconnected,
} DS_EventType;

typedef struct DS_Event {
    DS_EventType Type;
    union {
        struct {
            uint8_t Data;
        } ControlData;
    };
} DS_Event;

typedef int DS_Bool;
typedef int DS_Status;

#define DS_STATUS_SUCCESS 0
#define DS_ALREADY_INITIALIZED 1
#define DS_INVALID_PARAMETER 2
#define DS_OUT_OF_MEMORY 3

extern "C" {

DS_Status DS_Initialize(const DS_EventCallbacks* EventCallbacks);
void DS_Cleanup(void);

uint32_t DS_ReadEvents(DS_Event* Events, uint32_t EventCount, uint32_t* RemainingEvents);
void DS_FreeEvents(DS_Event* Events, uint32_t EventCount);

void SetTeam(const char* Name);

}
