#pragma once

#include <abcrypto.hpp>

namespace Crypto {

#define AES_IV_SIZE     16
#define AES_BLOCK_SIZE  16

class Aes
{
private:
    uint8_t iv_[AES_IV_SIZE];
    std::vector<uint8_t> blocks_;
    uint32_t numBlocks_;
    uint32_t size_;
    void SetIv();
    void PrefixIv();
    void ExtractIv();
    void Blockify(uint8_t* buffer, uint32_t len);
    void Reset();
public:
    size_t Encrypt(uint8_t* buffer, uint32_t len, uint8_t* out, uint32_t outlen, DH_KEY& key);
    size_t Decrypt(uint8_t* buffer, uint32_t len, uint8_t* out, uint32_t outlen, DH_KEY& key);

    static size_t AesEncrypt(uint8_t* buffer, uint32_t len, uint8_t* out, uint32_t outlen, DH_KEY& key);
    static size_t AesDecrypt(uint8_t* buffer, uint32_t len, uint8_t* out, uint32_t outlen, DH_KEY& key);

    static bool SelfTest();
};

}
