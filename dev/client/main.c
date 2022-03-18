#include "QuicConnection.h"
#include <stdlib.h>
#include <stdio.h>
#include "HPThread.h"
#include <unistd.h>

static
COMMLIB_API
void ConnReady(void* Context) {
    (void)Context;
    printf("Conn REady\n");
}

static
COMMLIB_API
void ConnDisconnect(void* Context) {
    (void)Context;
    printf("Conn Disconnect\n");
}

uint64_t LastTime = 0;

static
void
ThreadCallback(void* Context) {
    (void)Context;
    uint64_t Current = HPThread_GetTimeUs();
    printf("Delta %llu\n", Current - LastTime);
    LastTime = Current;
    //usleep(5000);
}

int main() {
    HPThread* Thread;
    CommLibStatus Status = HPThread_Initialize(ThreadCallback, NULL, 20, &Thread);
    printf("Thread Status %d\n", Status);

    QuicRegistration* Registration = NULL;
    Status = QC_GetRegistration("Client", 1, &Registration);
    printf("Reg Status %d\n", Status);

    QuicConnectionCallbacks Callbacks;
    Callbacks.ReadyCallback = ConnReady;
    Callbacks.DisconnectedCallback = ConnDisconnect;
    QuicConnection* Connection = NULL;
    Status = QC_CreateClientConnection(Registration, "localhost", 1360, (const uint8_t*)"frc", 3, 3, 0, &Callbacks, &Connection);
    printf("Conn Status %d\n", Status);

    getchar();

    printf("Past getchar\n");

    QC_ShutdownConnection(Connection);

    getchar();

    QC_FreeConnection(Connection);

    QC_FreeRegistration(Registration);
}
