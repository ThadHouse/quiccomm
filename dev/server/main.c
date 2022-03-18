#include "frccert.h"
#include "QuicConnection.h"
#include <stdlib.h>
#include <stdio.h>

static
COMMLIB_API
CommLibBoolean
ListenerCallback(QuicConnection* Connection, void* Context) {
    (void)Connection;
    (void)Context;
    printf("New Connection\n");
    return 0;
}

static COMMLIB_API
void ListenerStoppedCallback(void* Context) {
    (void)Context;
    printf("Listener Stopped\n");
}

int main() {
    uint32_t CertLength = 0;
    const char* Password = NULL;
    const uint8_t* Cert = QC_InternalGetCertificate(&CertLength, &Password);

    QuicRegistration* Registration = NULL;
    CommLibStatus Status = QC_GetRegistration("Server", 1, &Registration);
    printf("Reg Status %d\n", Status);

    QuicListener* Listener = NULL;
    QuicListenerCallbacks Callbacks;
    Callbacks.NewConnectionCallback = ListenerCallback;
    Callbacks.StoppedCallback = ListenerStoppedCallback;
    Callbacks.Context = NULL;
    Status = QC_CreateListener(Registration, 1360, (const uint8_t*)"frc", 3, Cert, CertLength, Password, 3, &Callbacks, &Listener);
    printf("Listener Status %d\n", Status);

    Status = QC_StartListener(Listener);
    printf("Listener Start Status %d\n", Status);

    getchar();

    QC_StopListener(Listener);

    getchar();

    QC_FreeListener(Listener);

    QC_FreeRegistration(Registration);
}
