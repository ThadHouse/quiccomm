#include "QuicApi.h"
#include "QuicConnection.h"

int main()
{
    qapi::QuicApi api{"DriverStation"};
    qapi::QuicConnection conn{"roborio-9999-frc.local", 9999};
    WPI_EventHandle handles[4];
    WPI_EventHandle triggeredHandles[4];
    printf("Waiting for event\n");
    handles[0] = conn.GetReadyEvent();
    handles[1] = conn.GetDisconnectedEvent();

    int timedOut = false;
    int signaled = WPI_WaitForObjectsTimeout(handles, 2, triggeredHandles, 25, &timedOut);

    printf("Event Signaled %d Timed Out %d\n", signaled, timedOut);

    for (int i = 0; i < signaled; i++) {
        if (triggeredHandles[i] == handles[0]) {
            printf("Connected\n");
        } else if (triggeredHandles[i] == handles[1]) {
            printf("Disconnected\n");
        }
    }
}
