#include "QuicConnection.h"
#include <stdlib.h>
#include <stdio.h>

static
QUIC_CONN_API
void ConnReady(void* Context) {
    (void)Context;
    printf("Conn REady\n");
}

static
QUIC_CONN_API
void ConnDisconnect(void* Context) {
    (void)Context;
    printf("Conn Disconnect\n");
}


int main() {
    QuicRegistration* Registration = NULL;
    QuicConnStatus Status = QC_GetRegistration("Client", 1, &Registration);
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
