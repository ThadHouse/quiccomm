#include "frccert.h"
#include "QuicConnection.h"
#include <stdlib.h>
#include <stdio.h>

int main() {
    uint32_t CertLength = 0;
    const char* Password = NULL;
    const uint8_t* Cert = QC_InternalGetCertificate(&CertLength, &Password);

    QuicRegistration* Registration = NULL;
    QuicConnStatus Status = QC_GetRegistration("Server", 1, &Registration);
    printf("Reg Status %d\n", Status);

    QuicListener* Listener = NULL;
    QuicListenerCallbacks Callbacks;
    Status = QC_CreateListener(Registration, 1360, (const uint8_t*)"frc", 3, Cert, CertLength, Password, 3, &Callbacks, &Listener);
    printf("Listener Status %d\n", Status);
}
