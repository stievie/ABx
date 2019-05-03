#pragma once

// https://github.com/thejinchao/dhexchange

#include <abcrypto.hpp>
#include <fstream>
#include <mutex>
#include <cassert>
#include <cstring>

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
        privateKey_{ 0 },
        publicKey_{ 0 },
        keysLoaded_(false)
    {}
    ~DHKeys() {}

    static void Test()
    {
        DH_KEY alice_private, alice_public;
        DH_generate_key_pair(alice_public, alice_private);
        DH_KEY bob_private, bob_public;
        DH_generate_key_pair(bob_public, bob_private);

        DH_KEY alice_secret;
        DH_generate_key_secret(alice_secret, alice_private, bob_public);
        DH_KEY bob_secret;
        DH_generate_key_secret(bob_secret, bob_private, alice_public);
        assert(memcmp(bob_secret, alice_secret, DH_KEY_LENGTH) == 0);

        DHKeys alice;
        DHKeys bob;
        alice.GenerateKeys();
        bob.GenerateKeys();

        DH_KEY aliceShared;
        alice.GetSharedKey(bob.GetPublickKey(), aliceShared);
        DH_KEY bobShared;
        bob.GetSharedKey(alice.GetPublickKey(), bobShared);
        assert(memcmp(aliceShared, bobShared, DH_KEY_LENGTH) == 0);
    }

    /// Generate new public and private keys
    void GenerateKeys()
    {
        // Random pool must be locked
        std::lock_guard<std::mutex> lockGuard(lock_);
        DH_generate_key_pair(publicKey_, privateKey_);
        keysLoaded_ = true;
    }
    bool LoadKeys(const std::string& fileName)
    {
        if (keysLoaded_)
            return false;

        std::ifstream f(fileName, std::ios::in | std::ios::binary);
        if (!f.is_open())
            return false;

        f.seekg(0, std::ios::end);
        std::streampos end = f.tellg();
        f.seekg(0, std::ios::beg);
        std::streampos start = f.tellg();
        int64_t size = end - start;
        if (size != DH_KEY_LENGTH * 2)
        {
            f.close();
            return false;
        }

        f.read((char*)&publicKey_, DH_KEY_LENGTH);
        f.read((char*)&privateKey_, DH_KEY_LENGTH);
        f.close();
        keysLoaded_ = true;
        return true;
    }
    bool SaveKeys(const std::string& fileName)
    {
        if (!keysLoaded_)
            return false;

        std::ofstream f(fileName, std::ios::binary);
        if (!f.is_open())
        {
            return false;
        }
        f.write((char*)&publicKey_, DH_KEY_LENGTH);
        f.write((char*)&privateKey_, DH_KEY_LENGTH);
        f.close();
        return true;
    }
    /// Get the shared key
    void GetSharedKey(const DH_KEY& publicKey, DH_KEY& sharedKey)
    {
        // Use our private key and the recipients public key to create a
        // shared key.
        DH_generate_key_secret(sharedKey, privateKey_, publicKey);
    }
    const DH_KEY& GetPublickKey() const { return publicKey_; }
};

}
