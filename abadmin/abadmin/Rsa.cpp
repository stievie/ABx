#include "stdafx.h"
#include "Rsa.h"

Rsa Rsa::Instance;

Rsa::Rsa()
{
    mpz_init2(mod_, 1024);
    mpz_init2(e_, 1024);
}

Rsa::~Rsa()
{
    mpz_clear(mod_);
    mpz_clear(e_);
}

void Rsa::SetPublicKey(char* m, const std::string& e)
{
    mpz_import(mod_, 128, 1, 1, 0, 0, m);
    mpz_set_str(e_, e.c_str(), 10);
}

bool Rsa::Encrypt(char* msg, long size)
{
    mpz_t plain, c;
    mpz_init2(plain, 1024);
    mpz_init2(c, 1024);

    mpz_import(plain, 128, 1, 1, 0, 0, msg);

    mpz_powm(c, plain, e_, mod_);

    size_t count = (mpz_sizeinbase(c, 2) + 7) / 8;
    memset(msg, 0, 128 - count);
    mpz_export(&msg[128 - count], NULL, 1, 1, 0, 0, c);

    mpz_clear(c);
    mpz_clear(plain);
    return true;
}

#pragma comment(lib, "mpir.lib")
