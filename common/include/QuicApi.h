#pragma once

#include <string>

namespace qapi {

class QuicApi {
public:
    explicit QuicApi(std::string registrationName);
    ~QuicApi() noexcept;
};

}
