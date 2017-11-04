#pragma once

#include <gmp.h>
#include <string>
#include <stdint.h>
#include <mutex>

namespace Crypto {

class Rsa
{
public:
    Rsa();
    ~Rsa();
    void SetKey(const char* p, const char* q, const char* d);
    bool SetKey(const std::string& file);
    bool Decrypt(char* msg, size_t size);
    bool Encrypt(char* msg, size_t size, const char* key);

    int32_t GetKeySize() const;
    void GetPublicKey(char* buffer);
private:
    std::recursive_mutex lock_;
    //use only GMP
    mpz_t m_p, m_q, m_u, m_d, m_dp, m_dq, m_mod;
public:
    static Rsa Instance;
};

}
