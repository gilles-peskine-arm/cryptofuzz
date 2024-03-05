#include "module.h"
#include <cryptofuzz/util.h>
#include <cryptofuzz/repository.h>
#include <fuzzing/datasource/id.hpp>
#include <mbedtls/platform.h>
#include <psa/crypto.h>

namespace cryptofuzz {
namespace module {

#define CF_ASSERT_PSA(expr) CF_ASSERT_EQ(expr, PSA_SUCCESS)

static bool is_cipher_consistent(SymmetricCipher cipher) {
    /* For some reason, modules can receive a cipher type that's
     * inconsistent with the key length, even though the module does not
     * have an interface to report that it correctly rejected an
     * invalid length. */
    switch (cipher.cipherType) {
    case CF_CIPHER("AES"):
        switch (cipher.key.GetSize()) {
        case 16:
        case 24:
        case 32:
            return true;
        default:
            return false;
        }

    case CF_CIPHER("AES_128_CBC"):
    case CF_CIPHER("AES_128_CBC_HMAC_SHA1"):
    case CF_CIPHER("AES_128_CBC_HMAC_SHA256"):
    case CF_CIPHER("AES_128_CCM"):
    case CF_CIPHER("AES_128_CFB"):
    case CF_CIPHER("AES_128_CFB1"):
    case CF_CIPHER("AES_128_CFB128"):
    case CF_CIPHER("AES_128_CFB8"):
    case CF_CIPHER("AES_128_CTR"):
    case CF_CIPHER("AES_128_ECB"):
    case CF_CIPHER("AES_128_GCM"):
    case CF_CIPHER("AES_128_OCB"):
    case CF_CIPHER("AES_128_OFB"):
    case CF_CIPHER("AES_128_WRAP"):
    case CF_CIPHER("AES_128_WRAP_PAD"):
        return cipher.key.GetSize() == 16;

    case CF_CIPHER("AES_128_XTS"):
        return cipher.key.GetSize() == 32;

    case CF_CIPHER("AES_192_CBC"):
    case CF_CIPHER("AES_192_CCM"):
    case CF_CIPHER("AES_192_CFB"):
    case CF_CIPHER("AES_192_CFB1"):
    case CF_CIPHER("AES_192_CFB128"):
    case CF_CIPHER("AES_192_CFB8"):
    case CF_CIPHER("AES_192_CTR"):
    case CF_CIPHER("AES_192_ECB"):
    case CF_CIPHER("AES_192_GCM"):
    case CF_CIPHER("AES_192_OFB"):
    case CF_CIPHER("AES_192_WRAP"):
    case CF_CIPHER("AES_192_WRAP_PAD"):
        return cipher.key.GetSize() == 24;

    case CF_CIPHER("AES_192_XTS"):
        return cipher.key.GetSize() == 48;

    case CF_CIPHER("AES_256_CBC"):
    case CF_CIPHER("AES_256_CBC_HMAC_SHA1"):
    case CF_CIPHER("AES_256_CCM"):
    case CF_CIPHER("AES_256_CFB"):
    case CF_CIPHER("AES_256_CFB1"):
    case CF_CIPHER("AES_256_CFB128"):
    case CF_CIPHER("AES_256_CFB8"):
    case CF_CIPHER("AES_256_CTR"):
    case CF_CIPHER("AES_256_ECB"):
    case CF_CIPHER("AES_256_GCM"):
    case CF_CIPHER("AES_256_OCB"):
    case CF_CIPHER("AES_256_OFB"):
    case CF_CIPHER("AES_256_WRAP"):
    case CF_CIPHER("AES_256_WRAP_PAD"):
        return cipher.key.GetSize() == 32;

    case CF_CIPHER("AES_256_XTS"):
        return cipher.key.GetSize() == 64;

    case CF_CIPHER("ARIA_128_CBC"):
    case CF_CIPHER("ARIA_128_CCM"):
    case CF_CIPHER("ARIA_128_CFB"):
    case CF_CIPHER("ARIA_128_CFB1"):
    case CF_CIPHER("ARIA_128_CFB128"):
    case CF_CIPHER("ARIA_128_CFB8"):
    case CF_CIPHER("ARIA_128_CTR"):
    case CF_CIPHER("ARIA_128_ECB"):
    case CF_CIPHER("ARIA_128_GCM"):
    case CF_CIPHER("ARIA_128_OFB"):
        return cipher.key.GetSize() == 16;

    case CF_CIPHER("ARIA_192_CBC"):
    case CF_CIPHER("ARIA_192_CCM"):
    case CF_CIPHER("ARIA_192_CFB"):
    case CF_CIPHER("ARIA_192_CFB1"):
    case CF_CIPHER("ARIA_192_CFB128"):
    case CF_CIPHER("ARIA_192_CFB8"):
    case CF_CIPHER("ARIA_192_CTR"):
    case CF_CIPHER("ARIA_192_ECB"):
    case CF_CIPHER("ARIA_192_GCM"):
    case CF_CIPHER("ARIA_192_OFB"):
        return cipher.key.GetSize() == 24;

    case CF_CIPHER("ARIA_256_CBC"):
    case CF_CIPHER("ARIA_256_CCM"):
    case CF_CIPHER("ARIA_256_CFB"):
    case CF_CIPHER("ARIA_256_CFB1"):
    case CF_CIPHER("ARIA_256_CFB128"):
    case CF_CIPHER("ARIA_256_CFB8"):
    case CF_CIPHER("ARIA_256_CTR"):
    case CF_CIPHER("ARIA_256_ECB"):
    case CF_CIPHER("ARIA_256_GCM"):
    case CF_CIPHER("ARIA_256_OFB"):
        return cipher.key.GetSize() == 32;

    case CF_CIPHER("CAMELLIA_128_CBC"):
    case CF_CIPHER("CAMELLIA_128_CFB"):
    case CF_CIPHER("CAMELLIA_128_CFB1"):
    case CF_CIPHER("CAMELLIA_128_CFB128"):
    case CF_CIPHER("CAMELLIA_128_CFB8"):
    case CF_CIPHER("CAMELLIA_128_CTR"):
    case CF_CIPHER("CAMELLIA_128_ECB"):
    case CF_CIPHER("CAMELLIA_128_GCM"):
    case CF_CIPHER("CAMELLIA_128_OFB"):
        return cipher.key.GetSize() == 16;

    case CF_CIPHER("CAMELLIA_192_CBC"):
    case CF_CIPHER("CAMELLIA_192_CFB"):
    case CF_CIPHER("CAMELLIA_192_CFB1"):
    case CF_CIPHER("CAMELLIA_192_CFB128"):
    case CF_CIPHER("CAMELLIA_192_CFB8"):
    case CF_CIPHER("CAMELLIA_192_CTR"):
    case CF_CIPHER("CAMELLIA_192_ECB"):
    case CF_CIPHER("CAMELLIA_192_GCM"):
    case CF_CIPHER("CAMELLIA_192_OFB"):
        return cipher.key.GetSize() == 24;

    case CF_CIPHER("CAMELLIA_256_CBC"):
    case CF_CIPHER("CAMELLIA_256_CFB"):
    case CF_CIPHER("CAMELLIA_256_CFB1"):
    case CF_CIPHER("CAMELLIA_256_CFB128"):
    case CF_CIPHER("CAMELLIA_256_CFB8"):
    case CF_CIPHER("CAMELLIA_256_CTR"):
    case CF_CIPHER("CAMELLIA_256_ECB"):
    case CF_CIPHER("CAMELLIA_256_GCM"):
    case CF_CIPHER("CAMELLIA_256_OFB"):
        return cipher.key.GetSize() == 32;

    case CF_CIPHER("CHACHA20"):
    case CF_CIPHER("CHACHA20_POLY1305"):
        return cipher.key.GetSize() == 32;

    case CF_CIPHER("DES_CBC"):
    case CF_CIPHER("DES_CFB"):
    case CF_CIPHER("DES_CFB1"):
    case CF_CIPHER("DES_CFB8"):
    case CF_CIPHER("DES_CTR"):
    case CF_CIPHER("DES_ECB"):
    case CF_CIPHER("DES_OFB"):
        return cipher.key.GetSize() == 8;

    case CF_CIPHER("DES_EDE"):
    case CF_CIPHER("DES_EDE_CBC"):
    case CF_CIPHER("DES_EDE_CFB"):
    case CF_CIPHER("DES_EDE_ECB"):
    case CF_CIPHER("DES_EDE_OFB"):
        return cipher.key.GetSize() == 16;

    case CF_CIPHER("DES3_CBC"):
    case CF_CIPHER("DES_EDE3"):
    case CF_CIPHER("DES_EDE3_CBC"):
    case CF_CIPHER("DES_EDE3_CFB"):
    case CF_CIPHER("DES_EDE3_CFB1"):
    case CF_CIPHER("DES_EDE3_CFB8"):
    case CF_CIPHER("DES_EDE3_ECB"):
    case CF_CIPHER("DES_EDE3_OFB"):
    case CF_CIPHER("DES_EDE3_WRAP"):
        return cipher.key.GetSize() == 24;

    default:
        return true;
    }
}

namespace TF_PSA_Crypto_detail {
    Datasource* ds;

    inline void SetGlobalDs(Datasource* ds) {
        TF_PSA_Crypto_detail::ds = ds;
    }

    inline void UnsetGlobalDs(void) {
        TF_PSA_Crypto_detail::ds = nullptr;
    }

    inline bool AllocationFailure(void) {
#if defined(CRYPTOFUZZ_MBEDTLS_ALLOCATION_FAILURES)
        bool fail = false;
        if ( ds == nullptr ) {
            return fail;
        }
        try {
            fail = ds->Get<bool>();
        } catch ( ... ) { }

        return fail;
#else
        return false;
#endif
    }
}

static void* mbedTLS_custom_calloc(size_t A, size_t B) {
    if ( TF_PSA_Crypto_detail::AllocationFailure() == true ) {
        return nullptr;
    }

    /* TODO detect overflows */
    const size_t size = A*B;
    void* p = util::malloc(size);
    if ( size ) {
        memset(p, 0x00, size);
    }
    return p;
}

static void mbedTLS_custom_free(void* ptr) {
    util::free(ptr);
}

TF_PSA_Crypto::TF_PSA_Crypto(void) :
    Module("PSA-Crypto") {

    if ( mbedtls_platform_set_calloc_free(mbedTLS_custom_calloc, mbedTLS_custom_free) != 0 ) {
        abort();
    }

    /* The most plausible error cause here is that Mbed TLS is compiled
     * with MBEDTLS_ENTROPY_NV_SEED enabled and the entropy seed file is
     * missing. To fix that problem, create a 64-byte file called "seedfile"
     * in the current directory. */
    CF_ASSERT_PSA(psa_crypto_init());
}

TF_PSA_Crypto::~TF_PSA_Crypto(void)
{
    mbedtls_psa_crypto_free();
}

namespace TF_PSA_Crypto_detail {

    psa_algorithm_t to_psa_algorithm_t(const component::DigestType& digestType) {
        using fuzzing::datasource::ID;

        static const std::map<uint64_t, psa_algorithm_t> LUT = {
            { CF_DIGEST("MD5"), PSA_ALG_MD5 },
            { CF_DIGEST("RIPEMD160"), PSA_ALG_RIPEMD160 },
            { CF_DIGEST("SHA1"), PSA_ALG_SHA_1 },
            { CF_DIGEST("SHA224"), PSA_ALG_SHA_224 },
            { CF_DIGEST("SHA256"), PSA_ALG_SHA_256 },
            { CF_DIGEST("SHA384"), PSA_ALG_SHA_384 },
            { CF_DIGEST("SHA512"), PSA_ALG_SHA_512 },
            { CF_DIGEST("SHA3-224"), PSA_ALG_SHA3_224 },
            { CF_DIGEST("SHA3-256"), PSA_ALG_SHA3_256 },
            { CF_DIGEST("SHA3-384"), PSA_ALG_SHA3_384 },
            { CF_DIGEST("SHA3-512"), PSA_ALG_SHA3_512 },
        };

        if ( LUT.find(digestType.Get()) == LUT.end() ) {
            return PSA_ALG_NONE;
        }

        return LUT.at(digestType.Get());
    }

    psa_key_type_t to_psa_key_type_t(const component::SymmetricCipherType& cipherType) {
        using fuzzing::datasource::ID;
        static const std::map<uint64_t, psa_key_type_t> LUT = {
            { CF_CIPHER("AES_128_ECB"), PSA_KEY_TYPE_AES },
            { CF_CIPHER("AES_192_ECB"), PSA_KEY_TYPE_AES },
            { CF_CIPHER("AES_256_ECB"), PSA_KEY_TYPE_AES },
            { CF_CIPHER("ARIA_128_ECB"), PSA_KEY_TYPE_AES },
            { CF_CIPHER("ARIA_192_ECB"), PSA_KEY_TYPE_ARIA },
            { CF_CIPHER("ARIA_256_ECB"), PSA_KEY_TYPE_ARIA },
            { CF_CIPHER("CAMELLIA_128_ECB"), PSA_KEY_TYPE_CAMELLIA },
            { CF_CIPHER("CAMELLIA_192_ECB"), PSA_KEY_TYPE_CAMELLIA },
            { CF_CIPHER("CAMELLIA_256_ECB"), PSA_KEY_TYPE_CAMELLIA },
        };
        if ( LUT.find(cipherType.Get()) == LUT.end() ) {
            return PSA_ALG_NONE;
        }
        return LUT.at(cipherType.Get());
    }

    class HashOperation {
        psa_hash_operation_t operation;

    public:
        HashOperation() {
            operation = PSA_HASH_OPERATION_INIT;
        }
        ~HashOperation() {
            psa_hash_abort(&operation);
        }

        psa_status_t setup(psa_algorithm_t alg) {
            return psa_hash_setup(&operation, alg);
        }
        psa_status_t update(const unsigned char *input, size_t input_length) {
            return psa_hash_update(&operation, input, input_length);
        }
        psa_status_t finish(unsigned char *output, size_t output_size,
                            size_t *output_length) {
            return psa_hash_finish(&operation, output, output_size, output_length);
        }
        psa_status_t verify(const unsigned char *hash, size_t hash_length) {
            return psa_hash_verify(&operation, hash, hash_length);
        }
    };

    class MACOperation {
        psa_mac_operation_t operation = PSA_MAC_OPERATION_INIT;
        psa_key_type_t key_type = PSA_KEY_TYPE_NONE;
        size_t key_bits = 0;
        psa_key_id_t key = PSA_KEY_ID_NULL;
        psa_algorithm_t alg = PSA_ALG_NONE;

    public:
        MACOperation() {
        }
        ~MACOperation() {
            psa_mac_abort(&operation);
            operation = PSA_MAC_OPERATION_INIT;
            key_type = PSA_KEY_TYPE_NONE;
            key_bits = 0;
            psa_destroy_key(key);
            key = PSA_KEY_ID_NULL;
            alg = PSA_ALG_NONE;
        }

        size_t length() {
            return PSA_MAC_LENGTH(key_type, key_bits, alg);
        }

        psa_status_t set_key(psa_key_type_t key_type_,
                             const unsigned char *key_data, size_t key_length,
                             psa_algorithm_t alg_) {
            key_type = key_type_;
            alg = alg_;
            key_bits = PSA_BYTES_TO_BITS(key_length);
            psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
            psa_set_key_type(&attributes, key_type);
            psa_set_key_usage_flags(&attributes,
                                    PSA_KEY_USAGE_SIGN_MESSAGE |
                                    PSA_KEY_USAGE_VERIFY_MESSAGE);
            psa_set_key_algorithm(&attributes, alg);
            return psa_import_key(&attributes, key_data, key_length, &key);
        }

        psa_status_t sign(const unsigned char *input, size_t input_length,
                          unsigned char *output, size_t output_size,
                          size_t *output_length) {
            return psa_mac_compute(key, alg, input, input_length,
                                   output, output_size, output_length);
        }
        psa_status_t verify(const unsigned char *input, size_t input_length,
                            const unsigned char *mac, size_t mac_length) {
            return psa_mac_verify(key, alg, input, input_length,
                                  mac, mac_length);
        }

        psa_status_t sign_start() {
            return psa_mac_sign_setup(&operation, key, alg);
        }
        psa_status_t verify_start() {
            return psa_mac_verify_setup(&operation, key, alg);
        }
        psa_status_t update(const unsigned char *input, size_t input_length) {
            return psa_mac_update(&operation, input, input_length);
        }
        psa_status_t sign_finish(unsigned char *output, size_t output_size,
                                 size_t *output_length) {
            return psa_mac_sign_finish(&operation, output, output_size, output_length);
        }
        psa_status_t verify_finish(const unsigned char *mac, size_t mac_length) {
            return psa_mac_verify_finish(&operation, mac, mac_length);
        }
    };

}

static std::optional<component::Digest> hash_compute(operation::Digest& op,
                                                     Datasource &ds,
                                                     psa_algorithm_t alg) {
    std::vector<uint8_t> md(PSA_HASH_LENGTH(alg));
    size_t length = 0;
    bool const multipart = ds.Get<bool>();
    if (multipart) {
        TF_PSA_Crypto_detail::HashOperation operation;
        /* Initialize */
        util::Multipart parts = util::ToParts(ds, op.cleartext);
        CF_ASSERT_PSA(operation.setup(alg));

        /* Process */
        for (const auto& part : parts) {
            CF_ASSERT_PSA(operation.update(part.first, part.second));
        }

        /* Finalize */
        CF_ASSERT_PSA(operation.finish(md.data(), md.size(), &length));
    } else {
        /* One-shot computation */
        CF_ASSERT_PSA(psa_hash_compute(alg,
                                       op.cleartext.GetPtr(&ds), op.cleartext.GetSize(),
                                       md.data(), md.size(), &length));
    }
    return component::Digest(md.data(), length);
}

static void hash_verify(operation::Digest& op,
                        Datasource &ds,
                        psa_algorithm_t alg,
                        std::vector<uint8_t> expected_md) {
    /* Biaise towards the expected size */
    bool const correct_size = ds.Get<bool>();
    std::vector<uint8_t> const verify_md =
        correct_size ? ds.GetData(0, expected_md.size(), expected_md.size()) :
        ds.GetData(0, 0, PSA_HASH_MAX_SIZE * 2);
    psa_status_t const expected_verify_status =
        verify_md == expected_md ? PSA_SUCCESS : PSA_ERROR_INVALID_SIGNATURE;

    bool const multipart = ds.Get<bool>();
    if (multipart) {
        TF_PSA_Crypto_detail::HashOperation operation;
        /* Initialize */
        util::Multipart parts = util::ToParts(ds, op.cleartext);
        CF_ASSERT_PSA(operation.setup(alg));

        /* Process */
        for (const auto& part : parts) {
            CF_ASSERT_PSA(operation.update(part.first, part.second));
        }//

        /* Finalize */
        CF_ASSERT_EQ(operation.verify(verify_md.data(), verify_md.size()),
                     expected_verify_status);
    } else {
        /* One-shot computation */
        CF_ASSERT_EQ(psa_hash_compare(alg,
                                      op.cleartext.GetPtr(&ds), op.cleartext.GetSize(),
                                      verify_md.data(), verify_md.size()),
                     expected_verify_status);
    }
}

std::optional<component::Digest> TF_PSA_Crypto::OpDigest(operation::Digest& op) {
    std::optional<component::Digest> ret = std::nullopt;
    Datasource ds(op.modifier.GetPtr(), op.modifier.GetSize());
    TF_PSA_Crypto_detail::SetGlobalDs(&ds);

    psa_algorithm_t const alg =
        TF_PSA_Crypto_detail::to_psa_algorithm_t(op.digestType);
    /* Skip unknown algorithms */
    CF_CHECK_NE(alg, PSA_ALG_NONE);

    ret = hash_compute(op, ds, alg);
    hash_verify(op, ds, alg, ret->Get());

end:
    TF_PSA_Crypto_detail::UnsetGlobalDs();

    return ret;
}

static std::optional<component::MAC> mac_compute(
        TF_PSA_Crypto_detail::MACOperation& operation,
        const component::Cleartext& cleartext,
        Datasource &ds) {
    std::vector<uint8_t> mac(operation.length());
    size_t length = 0;
    bool const multipart = ds.Get<bool>();
    if (multipart) {
        util::Multipart parts = util::ToParts(ds, cleartext);
        CF_ASSERT_PSA(operation.sign_start());
        for (const auto& part : parts) {
            CF_ASSERT_PSA(operation.update(part.first, part.second));
        }
        CF_ASSERT_PSA(operation.sign_finish(mac.data(), mac.size(), &length));
    } else {
        /* One-shot computation */
        CF_ASSERT_PSA(operation.sign(cleartext.GetPtr(&ds), cleartext.GetSize(),
                                     mac.data(), mac.size(), &length));
    }
    return component::MAC(mac.data(), length);
}

static void mac_verify(
        TF_PSA_Crypto_detail::MACOperation& operation,
        const component::Cleartext& cleartext,
        Datasource &ds,
        std::vector<uint8_t> expected_mac) {
    /* Biaise towards the expected size */
    bool const correct_size = ds.Get<bool>();
    std::vector<uint8_t> const verify_mac =
        correct_size ? ds.GetData(0, expected_mac.size(), expected_mac.size()) :
        ds.GetData(0, 0, PSA_HASH_MAX_SIZE * 2);
    psa_status_t const expected_verify_status =
        verify_mac == expected_mac ? PSA_SUCCESS : PSA_ERROR_INVALID_SIGNATURE;

    bool const multipart = ds.Get<bool>();
    if (multipart) {
        util::Multipart parts = util::ToParts(ds, cleartext);
        CF_ASSERT_PSA(operation.verify_start());
        for (const auto& part : parts) {
            CF_ASSERT_PSA(operation.update(part.first, part.second));
        }
        CF_ASSERT_EQ(operation.verify_finish(verify_mac.data(), verify_mac.size()),
                     expected_verify_status);
    } else {
        CF_ASSERT_EQ(operation.verify(cleartext.GetPtr(&ds), cleartext.GetSize(),
                                      verify_mac.data(), verify_mac.size()),
                     expected_verify_status);
    }
}

std::optional<component::MAC> TF_PSA_Crypto::OpHMAC(operation::HMAC& op) {
    std::optional<component::MAC> ret = std::nullopt;
    Datasource ds(op.modifier.GetPtr(), op.modifier.GetSize());
    TF_PSA_Crypto_detail::SetGlobalDs(&ds);

    psa_algorithm_t const hash_alg =
        TF_PSA_Crypto_detail::to_psa_algorithm_t(op.digestType);
    /* Skip unknown algorithms */
    CF_CHECK_NE(hash_alg, PSA_ALG_NONE);

    /* PSA does not allow empty keys */
    CF_CHECK_NE(op.cipher.key.GetSize(), 0);

    {
        TF_PSA_Crypto_detail::MACOperation operation;
        CF_ASSERT_PSA(operation.set_key(PSA_KEY_TYPE_HMAC,
                                        op.cipher.key.GetPtr(&ds),
                                        op.cipher.key.GetSize(),
                                        PSA_ALG_HMAC(hash_alg)));
        ret = mac_compute(operation, op.cleartext, ds);
        mac_verify(operation, op.cleartext, ds, ret->Get());
    }

end:
    TF_PSA_Crypto_detail::UnsetGlobalDs();

    return ret;
}

std::optional<component::MAC> TF_PSA_Crypto::OpCMAC(operation::CMAC& op) {
    std::optional<component::MAC> ret = std::nullopt;
    Datasource ds(op.modifier.GetPtr(), op.modifier.GetSize());
    TF_PSA_Crypto_detail::SetGlobalDs(&ds);

    CF_CHECK(is_cipher_consistent(op.cipher));

    psa_key_type_t const key_type =
        TF_PSA_Crypto_detail::to_psa_key_type_t(op.cipher.cipherType);
    /* Skip unknown ciphers */
    CF_CHECK_NE(key_type, PSA_KEY_TYPE_NONE);

    {
        TF_PSA_Crypto_detail::MACOperation operation;
        CF_ASSERT_PSA(operation.set_key(key_type,
                                        op.cipher.key.GetPtr(&ds),
                                        op.cipher.key.GetSize(),
                                        PSA_ALG_CMAC));
        ret = mac_compute(operation, op.cleartext, ds);
        mac_verify(operation, op.cleartext, ds, ret->Get());
    }

end:
    TF_PSA_Crypto_detail::UnsetGlobalDs();

    return ret;
}

} /* namespace module */
} /* namespace cryptofuzz */
