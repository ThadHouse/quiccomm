#include "netcomm.h"
#include "remoteconnection.h"

using namespace ncom;

int main(){
    Netcomm netcomm;
    auto future = netcomm.StartListener();
    if (future.wait_for(std::chrono::seconds(2)) == std::future_status::timeout) {
        printf("Timed out\n");
        return 1;
    }
    auto conn = future.get();
}
