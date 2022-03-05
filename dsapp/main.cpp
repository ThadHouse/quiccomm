#include "QuicApi.h"
#include "QuicConnection.h"
#include <thread>

int main()
{
    qapi::QuicApi api{"DriverStation"};
    qapi::QuicConnection server{9999};
    qapi::QuicConnection conn{"localhost", 9999};

    WPI_EventHandle handles[4];
    WPI_EventHandle triggeredHandles[4];
    printf("Waiting for event\n");
    handles[0] = conn.GetReadyEvent();
    handles[1] = conn.GetDisconnectedEvent();
    handles[2] = server.GetReadyEvent();
    handles[3] = server.GetDisconnectedEvent();

    std::this_thread::sleep_for(std::chrono::seconds(2));

    int timedOut = false;
    int signaled = WPI_WaitForObjectsTimeout(handles, 4, triggeredHandles, 25, &timedOut);

    printf("Event Signaled %d Timed Out %d\n", signaled, timedOut);

    for (int i = 0; i < signaled; i++) {
        if (triggeredHandles[i] == handles[0]) {
            printf("Connected (Client)\n");
        } else if (triggeredHandles[i] == handles[1]) {
            printf("Disconnected (Client)\n");
        } else if (triggeredHandles[i] == handles[2]) {
            printf("Connected (Server)\n");
        } else if (triggeredHandles[i] == handles[3]) {
            printf("Disconnected (Server)\n");
        }
    }
}
