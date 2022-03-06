#include "netcomm.h"

using namespace ncom;

int main(){
    std::vector<NETCOMM_Event> events;

    while (true) {
        printf("Starting loop\n");
        Netcomm netcomm;
        auto handle = netcomm.GetEventHandle();
        bool exitLoop = false;
        while (!exitLoop) {
            bool signaled = wpi::WaitForObject(handle);
            if (signaled) {

                netcomm.GetEvents(events);
                for (auto&& event : events) {
                    switch (event.Type) {
                        case NETCOMM_Event_Connected:
                        printf("Connected\n");
                        break;
                        case NETCOMM_Event_Disconnected:
                        printf("Disconnected\n");
                        exitLoop = true;
                        break;
                    }
                }
            }
        }
    }
}
