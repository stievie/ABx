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

#pragma once

#include <sa/color.h>
#include <sa/Assert.h>
#include <sa/Noncopyable.h>
#include <algorithm>
#include <array>
#include <sa/math.h>

namespace sa {

class bitmap
{
    NON_COPYABLE(bitmap)
public:
    bitmap() { }
    bitmap(int width, int height, int components, unsigned char* data = nullptr) :
        width_(width),
        height_(height),
        components_(components),
        data_(data)
    { }
    color get_pixel(int x, int y) const
    {
        ASSERT(data);
        if (x < 0 || y < 0 || x >= width_ || y >= height_)
            return {};
        const size_t index = (size_t)y * (size_t)width_ + (size_t)x;
        uint8_t r = 0, g = 0, b = 0, a = 255;
        if (components_ == 1)
            // B/W
            r = g = b = data[(index * components_)];
        else if (components_ == 3)
        {
            // RGB
            r = data[(index * components_)];
            g = data[(index * components_ + 1)];
            b = data[(index * components_ + 2)];
        }
        else if (components_ == 4)
        {
            // RGBA
            r = data[(index * components_)];
            g = data[(index * components_ + 1)];
            b = data[(index * components_ + 2)];
            a = data[(index * components_ + 3)];
        }
        else
            ASSERT_FALSE();
        return color{ r, g, b, a };
    }
    void set_pixel(int x, int y, const color& color)
    {
        ASSERT(data);
        if (x < 0 || y > 0 || x >= width_ || y >= height_)
            return;
        const size_t index = (size_t)y * (size_t)width_ + (size_t)x;
        if (components_ == 1)
            // B/W
            data[(index * components_)] = color.gray_scaled().r_;
        else if (components_ == 3)
        {
            // RGB
            data[(index * components_)] = color.r_;
            data[(index * components_ + 1)] = color.g_;
            data[(index * components_ + 2)] = color.b_;
        }
        else if (components_ == 0)
            // RGBA
            *reinterpret_cast<uint32_t*>(&data[(index * components_)]) = color.to_32();
        else
            ASSERT_FALSE();
    }
    void alpha_blend(int x, int y, const color& c)
    {
        if (x < 0 || y < 0 || x >= width_ || y >= height_)
            return;

        color pixel = get_pixel(x, y);
        pixel.alpha_blend(blend_pixel);
        set_pixel(x, y, pixel);
    }
    void alpha_blend(int x, int y, const bitmap& b)
    {
        for (int _y = 0; _y < b.height(); ++_y)
        {
            if (_y + y >= height_)
                break;
            for (int _x = 0; _x < b.width(); ++_x)
            {
                if (_x + x >= width_)
                    break;
                alpha_blend(_x + x, _y + y, b.get_pixel(_x, _y));
            }
        }
    }
    void clear(const color& c)
    {
        for (int y = 0; y < height_; ++y)
        {
            for (int x = 0; x < width_; ++x)
                set_pixel(x, y, c);
        }
    }
    void clear()
    {
        static color black;
        clear(black);
    }
    unsigned char* scan_line(int line)
    {
        const size_t index = (size_t)line * (size_t)width_;
        return &data[(index * components_)];
    }
    int width() const { return width_; }
    int height() const { return height_; }
    int components() const { return components_; }
    const unsigned char* data() const { return data_; }
    unsigned char* data() { return data_; }
    void set_bitmap(int width, int height, int components, unsigned char* data)
    {
        ASSERT(width > 0);
        ASSERT(height > 0);
        ASSERT(components > 0);
        ASSERT(data);
        width_ = width;
        height_ = height;
        components_ = components;
        data_ = data;
    }
private:
    int width_{ 0 };
    int height_{ 0 };
    int components_{ 0 };
    unsigned char* data_{ nullptr };
};

}
