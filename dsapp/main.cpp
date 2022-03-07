//#include "QuicApi.h"
//#include "QuicConnection.h"
#include <thread>
#include "frcds.h"
#include "wpi/timestamp.h"

int main()
{
    ds::DriverStation ds;
    ds.SetHostname("localhost");

    auto event = ds.GetEventHandle();

    bool hasConnection = true;
    std::vector<DS_Event> Events;

    auto start = wpi::Now();

    int count = 0;

    while (true) {
        if (hasConnection) {
            bool TimedOut = false;
            auto elapsedTime = wpi::Now() - start;
            uint64_t waitTime = 20000 - elapsedTime;
            if (elapsedTime > 20000) {
                waitTime = 0;
            }
            bool Signaled = wpi::WaitForObject(event, waitTime / (double)1e6, &TimedOut);
            start = wpi::Now();
            if (!Signaled) {
                goto SendData;
            }
        } else {
            if (!wpi::WaitForObject(event)) {
                continue;
            }
        }
        ds.GetEvents(Events);
        for(auto&& event : Events) {
            switch (event.Type)
            {
            case DS_Event_Connected:
                hasConnection = true;
                break;

            case DS_Event_Disconnected:
                printf("Received Disconnected event\n");
                hasConnection = false;
                ds.AckDisconnect();
                break;

            default:
                break;
            }
        }
SendData:
        if (hasConnection) {
            ds.SendControlPacket();
            ds.SendGameData();
            count++;
            if (count == 50) {
                count = 0;
                printf("Rtt %d\n", (int)ds.GetRtt());
            }
        }
    }

    std::this_thread::sleep_for(std::chrono::seconds(100));
    // qapi::QuicApi api{"DriverStation"};
    // qapi::QuicConnection server{9999};
    // qapi::QuicConnection conn{"localhost", 9999};

    // WPI_EventHandle handles[4];
    // WPI_EventHandle triggeredHandles[4];
    // printf("Waiting for event\n");
    // handles[0] = conn.GetReadyEvent();
    // handles[1] = conn.GetDisconnectedEvent();
    // handles[2] = server.GetReadyEvent();
    // handles[3] = server.GetDisconnectedEvent();

    // std::this_thread::sleep_for(std::chrono::seconds(2));

    // int timedOut = false;
    // int signaled = WPI_WaitForObjectsTimeout(handles, 4, triggeredHandles, 25, &timedOut);

    // printf("Event Signaled %d Timed Out %d\n", signaled, timedOut);

    // for (int i = 0; i < signaled; i++) {
    //     if (triggeredHandles[i] == handles[0]) {
    //         printf("Connected (Client)\n");
    //     } else if (triggeredHandles[i] == handles[1]) {
    //         printf("Disconnected (Client)\n");
    //     } else if (triggeredHandles[i] == handles[2]) {
    //         printf("Connected (Server)\n");
    //     } else if (triggeredHandles[i] == handles[3]) {
    //         printf("Disconnected (Server)\n");
    //     }
    // }
}
