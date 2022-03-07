#include "QuicApi.h"
#include "QuicApiInternal.h"
#include <stdexcept>
#include <cstring>

namespace qapi
{

    const QUIC_API_TABLE *MsQuic;
    static HQUIC Registration;

    HQUIC GetRegistration()
    {
        return Registration;
    }

    QuicApi::QuicApi(std::string registrationName)
    {
        QUIC_STATUS Status = MsQuicOpen2(&MsQuic);
        if (QUIC_FAILED(Status))
        {
            throw std::runtime_error("Failed to open msquic");
        }
        QUIC_REGISTRATION_CONFIG RegConfig;
        std::memset(&RegConfig, 0, sizeof(RegConfig));
        RegConfig.AppName = registrationName.c_str();

        Status = MsQuic->RegistrationOpen(&RegConfig, &Registration);
        if (QUIC_FAILED(Status))
        {
            MsQuicClose(MsQuic);
            MsQuic = nullptr;
            throw std::runtime_error("Failed to open registration");
        }
    }

    QuicApi::~QuicApi() noexcept
    {
        if (MsQuic)
        {
            MsQuic->RegistrationClose(Registration);
            MsQuicClose(MsQuic);
        }
    }

    const QUIC_API_TABLE* GetApiTable() {
        return MsQuic;
    }
}
