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

static void HandleStreamData(wpi::span<const QuicConnection::DataBuffer> Buffers) {
    (void)Buffers;
}

static void HandleControlStreamData(wpi::span<const QuicConnection::DataBuffer> Buffers) {
    (void)Buffers;
}

static void HandleDatagramData(const QuicConnection::DataBuffer& Buffer) {
    (void)Buffer;
}

void DsCommThread() {
    QuicConnection::Callbacks Callbacks {
        ReadyHandler,
        DisconnectHandler,
        HandleDatagramData,
        HandleStreamData,
        HandleControlStreamData
    };

    while (true) {
        printf("Waiting for connection\n");
        QuicConnection Connection{1360, Callbacks};
        wpi::WaitForObject(DisconnectEvent.GetHandle());
    }
}

int main() {
    QuicApi Api{"NetcommDaemon"};

    std::thread DsComm{[]{DsCommThread();}};

    DsComm.join();
}
