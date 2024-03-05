#pragma once

#include <cryptofuzz/components.h>
#include <cryptofuzz/module.h>
#include <optional>

namespace cryptofuzz {
namespace module {

class TF_PSA_Crypto : public Module {
    public:
        TF_PSA_Crypto(void);
        ~TF_PSA_Crypto(void);
        std::optional<component::Digest> OpDigest(operation::Digest& op) override;
        std::optional<component::MAC> OpHMAC(operation::HMAC& op) override;
};

} /* namespace module */
} /* namespace cryptofuzz */
