#pragma once

#include <gmp.h>
#include <string>

class Rsa
{
private:
    mpz_t mod_, e_;
protected:
    Rsa();
public:
    ~Rsa();

    void SetPublicKey(char* m, const std::string& e);
    bool Encrypt(char* msg, long size);

    static Rsa Instance;
};

