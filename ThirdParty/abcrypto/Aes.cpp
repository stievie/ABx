#include "Aes.hpp"

namespace Crypto {

// Buffer should be a multiple of 16 or it adds padding bytes.

void Aes::SetIv()
{
    // Use the wrapper because it lockes the pool
    Utils::Random::Instance.GetBuff(iv_, AES_IV_SIZE);
}

void Aes::PrefixIv()
{
    for (int i = AES_IV_SIZE - 1; i >= 0; i--)
    {
        blocks_.insert(blocks_.begin(), iv_[i]);
    }
}

void Aes::ExtractIv()
{
    for (uint32_t i = 0; i < AES_IV_SIZE; i++)
    {
        iv_[i] = blocks_[0];
        // Delete IV from the block
        blocks_.erase(blocks_.begin());
    }
}

void Aes::Blockify(uint8_t* buffer, uint32_t len)
{
    numBlocks_ = static_cast<uint32_t>(ceil(len / AES_BLOCK_SIZE));
    size_ = numBlocks_ * AES_BLOCK_SIZE;
    blocks_.clear();
    for (uint32_t i = 0; i < size_; i++)
    {
        if (i < len)
            blocks_.push_back(buffer[i]);
        else
            blocks_.push_back(0);
    }
}

void Aes::Reset()
{
    SetIv();
    blocks_.clear();
}

size_t Aes::Encrypt(uint8_t* buffer, uint32_t len, uint8_t* out, uint32_t outlen, DH_KEY& key)
{
    Blockify(buffer, len);
    SetIv();
    AES_CBC_encrypt_buffer16_ip(blocks_.data(), size_, key, iv_);
    PrefixIv();
    memcpy_s(out, outlen, blocks_.data(), blocks_.size());
    return blocks_.size();
}

size_t Aes::Decrypt(uint8_t* buffer, uint32_t len, uint8_t* out, uint32_t outlen, DH_KEY& key)
{

    Blockify(buffer, len);
    ExtractIv();
    AES_CBC_decrypt_buffer16_ip(blocks_.data(), size_, key, iv_);
    memcpy_s(out, outlen, blocks_.data(), blocks_.size());
    return blocks_.size();
}

size_t Aes::AesEncrypt(uint8_t* buffer, uint32_t len, uint8_t* out, uint32_t outlen, DH_KEY& key)
{
    Aes aes;
    return aes.Encrypt(buffer, len, out, outlen, key);
}

size_t Aes::AesDecrypt(uint8_t* buffer, uint32_t len, uint8_t* out, uint32_t outlen, DH_KEY& key)
{
    Aes aes;
    return aes.Decrypt(buffer, len, out, outlen, key);
}

bool Aes::SelfTest()
{
    Aes aes;
    uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
    uint8_t data[] = {
        0xf5, 0x8c, 0x4c, 0x04, 0xd6, 0xe5, 0xf1, 0xba, 0x77, 0x9e, 0xab, 0xfb, 0x5f, 0x7b, 0xfb, 0xd6,
        0x9c, 0xfc, 0x4e, 0x96, 0x7e, 0xdb, 0x80, 0x8d, 0x67, 0x9f, 0x77, 0x7b, 0xc6, 0x70, 0x2c, 0x7d,
        0x39, 0xf2, 0x33, 0x69, 0xa9, 0xd9, 0xba, 0xcf, 0xa5, 0x30, 0xe2, 0x63, 0x04, 0x23, 0x14, 0x61,
        0xb2, 0xeb, 0x05, 0xe2, 0xc3, 0x9b, 0xe9, 0xfc, 0xda, 0x6c, 0x19, 0x07, 0x8c, 0x6a, 0x9d, 0x1b,
        0x9c, 0xfc, 0x4e, 0x96, 0x7e, 0xdb, 0x80, 0x8d, 0x67, 0x9f, 0x77, 0x7b, 0xc6, 0x70, 0x2c, 0x7d,
    };
    uint8_t enc[100];
    uint8_t out[100];

    size_t size = aes.Encrypt(data, sizeof(data), enc, 100, key);
    aes.Reset();
    aes.Decrypt(enc, (uint32_t)size, out, 100, key);
    if (memcmp((char*)out, (char*)data, sizeof(data)) == 0)
        return true;
    return false;
}

}
