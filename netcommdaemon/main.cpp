#include "QuicConnection.h"
#include "QuicApi.h"
#include "wpi/Synchronization.h"
#include <thread>

// Forward every time we get a control packet

using namespace qapi;


static wpi::Event DisconnectEvent;
static void ReadyHandler() {
    printf("Connected!\n");
}

static void DisconnectHandler() {
    printf("Disconnected\n");
    DisconnectEvent.Set();
}

static void HandleStreamData(wpi::span<const qapi::DataBuffer> Buffers) {
    (void)Buffers;
}

static void HandleControlStreamData(wpi::span<const qapi::DataBuffer> Buffers) {
    (void)Buffers;
}

static void HandleDatagramData(const qapi::DataBuffer& Buffer) {
    (void)Buffer;
}

void DsCommThread() {
    qapi::Callbacks Callbacks {
        ReadyHandler,
        DisconnectHandler,
        HandleDatagramData,
        HandleStreamData,
        HandleControlStreamData
    };

    while (true) {
        printf("Waiting for connection\n");
        int32_t Status = 0;
        QuicConnection Connection{1360, Callbacks, &Status};
        wpi::WaitForObject(DisconnectEvent.GetHandle());
    }
}

int main() {
    QuicApi Api{"NetcommDaemon"};

    std::thread DsComm{[]{DsCommThread();}};

    DsComm.join();
}
