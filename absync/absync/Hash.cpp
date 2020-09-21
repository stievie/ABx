/**
 * Copyright 2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "Hash.h"
extern "C" {
#include <abcrypto/sha1.h>
}
#include <fstream>
#include <vector>
#include <iomanip>
#include <sstream>

namespace Sync {

void HashFile(const std::string& filename, const std::string& outfile)
{
    std::ifstream ifs(filename, std::ifstream::binary);
    ifs.seekg(0, std::ios::end);
    size_t fileSize = static_cast<size_t>((long)ifs.tellg());
    if (fileSize == 0)
        return;

    Sha1Hash sha_hash;
    sha1_ctx ctx;
    sha1_init(&ctx);

    ifs.seekg(0, std::ios::beg);
    std::vector<char> buffer(4096);
    while (ifs)
    {
        ifs.read(&buffer[0], 4096);
        auto readLength = ifs.gcount();
        sha1_update(&ctx, (const unsigned char*)&buffer[0], (long)readLength);
    }
    sha1_final(&ctx, sha_hash.data());

    std::ofstream out(outfile, std::ios::out);
    out << sha_hash;
}

Sha1Hash GetFileHash(const std::string& filename)
{
    std::ifstream ifs(filename, std::ios::binary);
    ifs.seekg(0, std::ios::end);
    size_t fileSize = static_cast<size_t>((long)ifs.tellg());
    if (fileSize == 0)
        return {};

    Sha1Hash sha_hash;
    sha1_ctx ctx;
    sha1_init(&ctx);

    ifs.seekg(0, std::ios::beg);
    std::vector<char> buffer(4096);
    while (ifs)
    {
        ifs.read(&buffer[0], 4096);
        auto readLength = ifs.gcount();
        sha1_update(&ctx, (const unsigned char*)&buffer[0], (long)readLength);
    }
    sha1_final(&ctx, sha_hash.data());
    return sha_hash;
}

}