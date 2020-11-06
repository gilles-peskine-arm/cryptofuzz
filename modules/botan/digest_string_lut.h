static const std::map<uint64_t, std::string> LUT = {
    { CF_DIGEST("ADLER32"), "Adler32" },
    { CF_DIGEST("BLAKE2B160"), "Blake2b(160)" },
    { CF_DIGEST("BLAKE2B256"), "Blake2b(256)" },
    { CF_DIGEST("BLAKE2B384"), "Blake2b(384)" },
    { CF_DIGEST("BLAKE2B512"), "Blake2b(512)" },
    { CF_DIGEST("CRC32-RFC2440"), "CRC24" },
    { CF_DIGEST("CRC32"), "CRC32" },
    { CF_DIGEST("GOST-R-34.11-94"), "GOST-R-34.11-94" },
    { CF_DIGEST("MD4"), "MD4" },
    { CF_DIGEST("MD5"), "MD5" },
    { CF_DIGEST("RIPEMD160"), "RIPEMD-160" },
    { CF_DIGEST("SHA1"), "SHA1" },
    { CF_DIGEST("SHA224"), "SHA-224" },
    { CF_DIGEST("SHA256"), "SHA-256" },
    { CF_DIGEST("SHA384"), "SHA-384" },
    { CF_DIGEST("SHA512"), "SHA-512" },
    { CF_DIGEST("SHA3-224"), "SHA-3(224)" },
    { CF_DIGEST("SHA3-256"), "SHA-3(256)" },
    { CF_DIGEST("SHA3-384"), "SHA-3(384)" },
    { CF_DIGEST("SHA3-512"), "SHA-3(512)" },
    { CF_DIGEST("SKEIN_512"), "Skein-512" },
    { CF_DIGEST("SM3"), "SM3" },
    { CF_DIGEST("STREEBOG-256"), "Streebog-256" },
    { CF_DIGEST("STREEBOG-512"), "Streebog-512" },
    { CF_DIGEST("WHIRLPOOL"), "Whirlpool" },
    { CF_DIGEST("SHA512-256"), "SHA-512-256" },
    { CF_DIGEST("SHAKE128"), "SHAKE-128(128)" },
    { CF_DIGEST("SHAKE256"), "SHAKE-256(256)" },
    { CF_DIGEST("KECCAK_224"), "Keccak-1600(224)" },
    { CF_DIGEST("KECCAK_256"), "Keccak-1600(256)" },
    { CF_DIGEST("KECCAK_384"), "Keccak-1600(384)" },
    { CF_DIGEST("KECCAK_512"), "Keccak-1600(512)" },
    { CF_DIGEST("SIPHASH64"), "SipHash" },
};
