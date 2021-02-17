#include "module.h"
#include <cryptofuzz/util.h>
#include <cryptofuzz/repository.h>

extern "C" {
    #include <libsig.h>
}

namespace cryptofuzz {
namespace module {
namespace libecc_detail {
    Datasource* global_ds = nullptr;
    FILE* fp_dev_urandom = nullptr;
	const ec_sig_mapping *sm;

    std::map<uint64_t, const ec_str_params*> curveLUT;

    static void AddCurve(const uint64_t curveID, const std::string& curveName) {
        const ec_str_params *curve_params = ec_get_curve_params_by_name((const u8*)curveName.c_str(), curveName.size() + 1);

        CF_ASSERT(curve_params != nullptr, "Cannot initialize curve");

        curveLUT[curveID] = curve_params;
    }

    static const ec_str_params* GetCurve(const component::CurveType& curveType) {
        if ( curveLUT.find(curveType.Get()) == curveLUT.end() ) {
            return nullptr;
        }

        return curveLUT.at(curveType.Get());
    }
    std::optional<component::BignumPair> To_Component_BignumPair(const ec_pub_key& pub) {
        std::optional<component::BignumPair> ret = std::nullopt;

        uint8_t* out = nullptr;
        const size_t outSize = EC_PUB_KEY_EXPORT_SIZE(&pub);
        CF_ASSERT((outSize % 2) == 0, "Public key byte size is not even");
        CF_ASSERT((outSize % 3) == 0, "Public key byte size is not multiple of 3");
        out = util::malloc(outSize);
        CF_CHECK_EQ(ec_pub_key_export_to_buf(&pub, out, outSize), 0);
        {
            const size_t pointSize = outSize / 3;
            const auto X = util::BinToDec(out, pointSize);
            const auto Y = util::BinToDec(out + pointSize, pointSize);

            ret = {X, Y};
        }

end:
        util::free(out);

        return ret;
    }
}
}
}

extern "C" int get_random(unsigned char *buf, u16 len) {
    CF_ASSERT(cryptofuzz::module::libecc_detail::global_ds != nullptr, "Global datasource is NULL");

    if ( len == 0 ) {
        return 0;
    }

    try {
        const auto data = cryptofuzz::module::libecc_detail::global_ds->GetData(0, len, len);
        memcpy(buf, data.data(), len);
        return 0;
    } catch ( fuzzing::datasource::Datasource::OutOfData ) { }

    CF_ASSERT(fread(buf, len, 1, cryptofuzz::module::libecc_detail::fp_dev_urandom) == 1, "Reading from /dev/urandom failed");

    return 0;
}

namespace cryptofuzz {
namespace module {

libecc::libecc(void) :
    Module("libecc") {
    CF_ASSERT((libecc_detail::fp_dev_urandom = fopen("/dev/urandom", "rb")) != NULL, "Failed to open /dev/urandom");
    CF_ASSERT((libecc_detail::sm = get_sig_by_name("ECDSA")) != nullptr, "Cannot initialize ECDSA");

    /* Load curves */
    libecc_detail::AddCurve(CF_ECC_CURVE("brainpool224r1"), "BRAINPOOLP224R1");
    libecc_detail::AddCurve(CF_ECC_CURVE("brainpool256r1"), "BRAINPOOLP256R1");
    libecc_detail::AddCurve(CF_ECC_CURVE("brainpool384r1"), "BRAINPOOLP384R1");
    libecc_detail::AddCurve(CF_ECC_CURVE("brainpool512r1"), "BRAINPOOLP512R1");
    libecc_detail::AddCurve(CF_ECC_CURVE("secp192r1"), "SECP192R1");
    libecc_detail::AddCurve(CF_ECC_CURVE("secp224r1"), "SECP224R1");
    libecc_detail::AddCurve(CF_ECC_CURVE("secp256r1"), "SECP256R1");
    libecc_detail::AddCurve(CF_ECC_CURVE("secp384r1"), "SECP384R1");
    libecc_detail::AddCurve(CF_ECC_CURVE("secp521r1"), "SECP521R1");

    /* TODO */
#if 0
    "FRP256V1"
    "GOST256"
    "GOST512"
#endif
}

std::optional<component::ECC_PublicKey> libecc::OpECC_PrivateToPublic(operation::ECC_PrivateToPublic& op) {
    std::optional<component::ECC_PublicKey> ret = std::nullopt;
    Datasource ds(op.modifier.GetPtr(), op.modifier.GetSize());

    libecc_detail::global_ds = &ds;

    ec_priv_key priv;
	ec_pub_key pub;
    uint8_t* out = nullptr;
	ec_params params;
    ec_sig_alg_type sig_type;
    std::optional<std::vector<uint8_t>> priv_bytes;
    const ec_str_params* curve_params;

    /* Load curve */
    CF_CHECK_NE(curve_params = libecc_detail::GetCurve(op.curveType), nullptr);
    /* noret */ import_params(&params, curve_params);

    sig_type = libecc_detail::sm->type;

    {
        const auto priv_str = op.priv.ToTrimmedString();
        CF_CHECK_NE(priv_str, "0");
        CF_CHECK_NE(priv_str, *cryptofuzz::repository::ECC_CurveToOrder(op.curveType.Get()));
        CF_CHECK_NE(priv_bytes = util::DecToBin(priv_str), std::nullopt);
        CF_CHECK_LTE(priv_bytes->size(), NN_MAX_BYTE_LEN);
        /* noret */ ec_priv_key_import_from_buf(&priv, &params, priv_bytes->data(), priv_bytes->size(), sig_type);
        memset(&pub, 0, sizeof(pub));
        CF_CHECK_EQ(init_pubkey_from_privkey(&pub, &priv), 0);
        CF_CHECK_EQ(pub.magic, PUB_KEY_MAGIC);

        aff_pt Q_aff;
        prj_pt_to_aff(&Q_aff, &pub.y);
        ec_shortw_aff_to_prj(&pub.y, &Q_aff);
    }

    {
        const auto _ret = libecc_detail::To_Component_BignumPair(pub);
        CF_CHECK_TRUE(op.priv.IsLessThan(*cryptofuzz::repository::ECC_CurveToOrder(op.curveType.Get())));
        ret = _ret;
    }

end:
    util::free(out);

    libecc_detail::global_ds = nullptr;
    return ret;
}

std::optional<component::ECDSA_Signature> libecc::OpECDSA_Sign(operation::ECDSA_Sign& op) {
    if ( op.UseRandomNonce() == false && op.UseSpecifiedNonce() == false ) {
        return std::nullopt;
    }

    if ( op.digestType.Get() != CF_DIGEST("NULL") ) {
        return std::nullopt;
    }

    std::optional<component::ECDSA_Signature> ret = std::nullopt;
    Datasource ds(op.modifier.GetPtr(), op.modifier.GetSize());
    libecc_detail::global_ds = &ds;

    const ec_str_params* curve_params;
    std::optional<std::vector<uint8_t>> priv_bytes;
	ec_params params;
	ec_key_pair kp;
    struct ec_sign_context ctx;
    size_t signature_size = 0;
    uint8_t* signature = nullptr;
    std::optional<std::vector<uint8_t>> nonce_bytes;

    /* Load curve */
    CF_CHECK_NE(curve_params = libecc_detail::GetCurve(op.curveType), nullptr);
    /* noret */ import_params(&params, curve_params);

    {
        const auto priv_str = op.priv.ToTrimmedString();
        CF_CHECK_NE(priv_str, "0");
        CF_CHECK_NE(priv_str, *cryptofuzz::repository::ECC_CurveToOrder(op.curveType.Get()));
        CF_CHECK_NE(priv_bytes = util::DecToBin(priv_str), std::nullopt);
        CF_CHECK_LTE(priv_bytes->size(), NN_MAX_BYTE_LEN);
        CF_CHECK_EQ(ec_key_pair_import_from_priv_key_buf(&kp,
                    &params,
                    priv_bytes->data(), priv_bytes->size(),
                    ECDSA), 0);
    }

    signature_size = ECDSA_SIGLEN(kp.priv_key.params->ec_gen_order_bitlen);
    CF_ASSERT((signature_size % 2) == 0, "Signature size is not multiple of 2");
    signature = util::malloc(signature_size);

    CF_CHECK_EQ(ec_sign_init(&ctx, &kp, ECDSA, SHA256), 0);

    if ( op.UseSpecifiedNonce() == true ) {
        /* ecdsa_sign_raw crashes if nonce is 0 */
        CF_CHECK_NE(op.nonce.ToTrimmedString(), "0");

        CF_CHECK_NE(nonce_bytes = util::DecToBin(op.nonce.ToTrimmedString()), std::nullopt);
    }

    CF_CHECK_EQ(ecdsa_sign_raw(
                &ctx,
                op.cleartext.GetPtr(), op.cleartext.GetSize(),
                signature, signature_size,
                op.UseSpecifiedNonce() ? nonce_bytes->data() : nullptr,
                op.UseSpecifiedNonce() ? nonce_bytes->size() : 0), 0);
    {
        const auto pub = libecc_detail::To_Component_BignumPair(kp.pub_key);
        CF_CHECK_NE(pub, std::nullopt);

        ret = {
            {
                util::BinToDec(signature, signature_size / 2),
                util::BinToDec(signature + (signature_size / 2), signature_size / 2),
            },
            *pub
        };
    }

end:
    util::free(signature);

    libecc_detail::global_ds = nullptr;
    return ret;
}

} /* namespace module */
} /* namespace cryptofuzz */
