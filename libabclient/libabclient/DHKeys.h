#pragma once

#include <abcrypto.hpp>
#include <string>
#include <mutex>

namespace Crypto {

class DHKeys
{
private:
    DH_KEY privateKey_;
    DH_KEY publicKey_;
    bool keysLoaded_;
    std::mutex lock_;
public:
    DHKeys() :
        keysLoaded_(false)
    {}
    ~DHKeys() {}

    /// Generate new public and private keys
    void GenerateKeys();
    bool LoadKeys(const std::string& fileName);
    bool SaveKeys(const std::string& fileName);
    /// Get the shared key
    void GetSharedKey(const DH_KEY& publicKey, DH_KEY& sharedKey);

    static DHKeys Instance;
};

}
