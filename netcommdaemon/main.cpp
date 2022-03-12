#include "QuicConnection.h"
#include "QuicApi.h"
#include "wpi/Synchronization.h"

// Forward every time we get a control packet

using namespace qapi;

static void ReadyEvent() {

}

static void DisconnectEvent() {

}

static void HandleStreamData(wpi::span<const QuicConnection::DataBuffer> Buffers) {
    // Nothing in the Stream is useful
    (void)Buffers;
}

static void HandleControlStreamData(wpi::span<const QuicConnection::DataBuffer> Buffers) {
    (void)Buffers;
}

static void HandleDatagramData(const QuicConnection::DataBuffer& Buffer) {
    (void)Buffer;
}

int main() {


    QuicApi Api{"NetcommDaemon"};
    QuicConnection::Callbacks Callbacks {
        ReadyEvent,
        DisconnectEvent,
        HandleDatagramData,
        HandleStreamData,
        HandleControlStreamData
    };
    QuicConnection Connection{1360, Callbacks};


}
