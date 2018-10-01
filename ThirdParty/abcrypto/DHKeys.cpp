#include "DHKeys.hpp"
#include <fstream>

namespace Crypto {

void DHKeys::GenerateKeys()
{
    // Random pool must be locked
    std::lock_guard<std::mutex> lockGuard(lock_);
    DH_generate_key_pair(publicKey_, privateKey_);
    keysLoaded_ = true;
}

bool DHKeys::LoadKeys(const std::string& fileName)
{
    if (keysLoaded_)
        return false;

    std::ifstream f(fileName, std::ios::in || std::ios::binary);
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

bool DHKeys::SaveKeys(const std::string& fileName)
{
    if (!keysLoaded_)
        return false;

    std::ofstream f(fileName);
    if (!f.is_open())
    {
        return false;
    }
    f.write((char*)&publicKey_, DH_KEY_LENGTH);
    f.write((char*)&privateKey_, DH_KEY_LENGTH);
    f.close();
    return true;
}

void DHKeys::GetSharedKey(const DH_KEY& publicKey, DH_KEY& sharedKey)
{
    // Use our private key and the recipients public key to create a
    // shared key.
    DH_generate_key_secret(sharedKey, privateKey_, publicKey);
}

}
