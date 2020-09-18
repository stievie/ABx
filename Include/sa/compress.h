/**
 * Copyright Nick
 * https://codereview.stackexchange.com/questions/141623/zlib-wrapper-class
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

#pragma once

#include <vector>
#include <sa/Noncopyable.h>
#ifdef SA_ZLIB_SUPPORT
#include <zlib.h>
#endif
#ifdef SA_LZ4_SUPPORT
#include <lz4.h>
#endif

namespace sa {

#ifdef SA_ZLIB_SUPPORT
namespace details {

enum class zlib_op
{
    compress,
    decompress
};

template<zlib_op OP>
struct zlib_opbase;

template<>
struct zlib_opbase<zlib_op::compress>
{
    static bool init(z_stream* zs, int level)
    {
        return deflateInit(zs, level) == Z_OK;
    }
    static int done(z_stream* zs)
    {
        return deflateEnd(zs);
    }
    static int reset(z_stream* zs)
    {
        return deflateReset(zs);
    }
    static int process(z_stream* zs)
    {
        return deflate(zs, Z_FINISH);
    }
};

template<>
struct zlib_opbase<zlib_op::decompress> {
    static bool init(z_stream* zs, int const)
    {
        return inflateInit(zs) == Z_OK;
    }
    static int done(z_stream* zs)
    {
        return inflateEnd(zs);
    }
    static int reset(z_stream* zs)
    {
        return inflateReset(zs);
    }
    static int process(z_stream* zs)
    {
        return inflate(zs, Z_FINISH);
    }
};

template<zlib_op OP>
struct zlib_ops
{
    static void init(z_stream* zs, int level)
    {
        int result = zlib_opbase<OP>::init(zs, level);
        if (!result)
        {
            std::bad_alloc exception;
            throw exception;
        }
    }
    static int done(z_stream* zs)
    {
        return zlib_opbase<OP>::done(zs);
    }

    static bool process(z_stream* zs, const char* data, size_t const size, char* data_out, size_t& size_out)
    {
        if (data == nullptr || size == 0 || data_out == nullptr || size_out == 0)
            return false;

        zlib_opbase<OP>::reset(zs);

        zs->next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data));
        zs->avail_in = static_cast<uInt>(size);

        zs->next_out = reinterpret_cast<Bytef*>(data_out);
        zs->avail_out = static_cast<uInt>(size_out);

        int result = zlib_opbase<OP>::process(zs);

        size_out = zs->total_out;

        return result == Z_STREAM_END;
    }
};

}

class zlib_base
{
    NON_COPYABLE(zlib_base)
    NON_MOVEABLE(zlib_base)
protected:
    zlib_base() { }
};

class zlib_compress : public zlib_base
{
public:
    const static int NO_COMPRESSION = Z_NO_COMPRESSION;
    const static int BEST_SPEED = Z_BEST_SPEED;
    const static int BEST_COMPRESSION = Z_BEST_COMPRESSION;
    const static int DEFAULT_COMPRESSION = Z_DEFAULT_COMPRESSION;
private:
    z_stream buffer_{};
public:
    zlib_compress(int level = DEFAULT_COMPRESSION)
    {
        details::zlib_ops<details::zlib_op::compress>::init(&buffer_, level);
    }
    ~zlib_compress()
    {
        details::zlib_ops<details::zlib_op::compress>::done(&buffer_);
    }
    bool operator()(const std::vector<char>& in, std::vector<char>& out, size_t& out_size)
    {
        out_size = out.size();
        bool ret = details::zlib_ops<details::zlib_op::compress>::process(&buffer_, &in[0], in.size(), &out[0], out_size);
        if (ret)
            out.resize(out_size);
        return ret;
    }
    bool operator()(const char* in, size_t in_size, char* out, size_t& out_size)
    {
        return details::zlib_ops<details::zlib_op::compress>::process(&buffer_, in, in_size, out, out_size);
    }
};

class zlib_decompress : public zlib_base
{
private:
    z_stream buffer_{};
public:
    zlib_decompress()
    {
        details::zlib_ops<details::zlib_op::decompress>::init(&buffer_, 0);
    }
    ~zlib_decompress()
    {
        details::zlib_ops<details::zlib_op::decompress>::done(&buffer_);
    }
    bool operator()(const std::vector<char>& in, std::vector<char>& out, size_t& out_size)
    {
        out_size = out.size();
        bool ret = details::zlib_ops<details::zlib_op::decompress>::process(&buffer_, &in[0], in.size(), &out[0], out_size);
        if (ret)
            out.resize(out_size);
        return ret;
    }
    bool operator()(const char* in, size_t in_size, char* out, size_t& out_size)
    {
        return details::zlib_ops<details::zlib_op::decompress>::process(&buffer_, in, in_size, out, out_size);
    }
};
#endif

#ifdef SA_LZ4_SUPPORT
class lz4_compress
{
public:
    bool operator()(const std::vector<char>& in, std::vector<char>& out, size_t& out_size)
    {
        int size = (int)out.size();
        int ret = LZ4_compress_default(&in[0], &out[0], (int)in.size(), size);
        if (ret > 0)
        {
            out_size = ret;
            out.resize(out_size);
            return true;
        }
        return false;
    }
    bool operator()(const char* in, size_t in_size, char* out, size_t& out_size)
    {
        int size = (int)out_size;
        int ret = LZ4_compress_default(in, out, (int)in_size, size);
        if (ret > 0)
        {
            out_size = ret;
            return true;
        }
        return false;
    }
};

class lz4_decompress
{
public:
    bool operator()(const std::vector<char>& in, std::vector<char>& out, size_t& out_size)
    {
        int size = (int)out.size();
        int ret = LZ4_decompress_safe(&in[0], &out[0], (int)in.size(), size);
        if (ret > 0)
        {
            out_size = ret;
            out.resize(out_size);
            return true;
        }
        return false;
    }
    bool operator()(const char* in, size_t in_size, char* out, size_t& out_size)
    {
        int size = (int)out_size;
        int ret = LZ4_decompress_safe(in, out, (int)in_size, size);
        if (ret > 0)
        {
            out_size = ret;
            return true;
        }
        return false;
    }
};
#endif

}
