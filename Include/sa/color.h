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

#include <stdint.h>
#include <tuple>
#include <limits>
#include <string>
#include <string_view>
#include <sstream>
#include <charconv>
#include <iomanip>
#include <algorithm>

// HSL, HSV <-> RGB
// https://github.com/ratkins/RGBConverter/blob/master/RGBConverter.cpp
// https://en.wikipedia.org/wiki/HSL_and_HSV
namespace sa {

namespace details {

inline float hue2rgb(float p, float q, float t)
{
    if (t < 0.0f) t += 1.0f;
    if (t > 1.0f) t -= 1.0f;
    if (t < 1.0f / 6.0f)
        return p + (q - p) * 6.0f * t;
    if (t < 1.0f / 2.0f)
        return q;
    if (t < 2.0f / 3.0f)
        return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
    return p;
}

template <typename T>
inline bool equals(T lhs, T rhs)
{
    return lhs + std::numeric_limits<T>::epsilon() >= rhs &&
        lhs - std::numeric_limits<T>::epsilon() <= rhs;
}

}

class color
{
public:
    // r, g, b, a in range 0..1
    static color from_rgb(float r, float g, float b, float a = 1.0f)
    {
        return { static_cast<uint8_t>(std::clamp(r, 0.0f, 1.0f) * 255.0f),
            static_cast<uint8_t>(std::clamp(g, 0.0f, 1.0f) * 255.0f),
            static_cast<uint8_t>(std::clamp(b, 0.0f, 1.0f) * 255.0f),
            static_cast<uint8_t>(std::clamp(a, 0.0f, 1.0f) * 255.0f) };
    }
    static color from_32(uint32_t value)
    {
        const int a = (value >> 24u) & 0xffu;
        const int b = (value >> 16u) & 0xffu;
        const int g = (value >> 8u) & 0xffu;
        const int r = (value >> 0u) & 0xffu;
        return { (uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a };
    }
    static color from_24(uint32_t value)
    {
        const int a = 255;
        const int b = (value >> 16u) & 0xffu;
        const int g = (value >> 8u) & 0xffu;
        const int r = (value >> 0u) & 0xffu;
        return { (uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a };
    }
    // h, s, l must be in 0..1
    static color from_hsl(float h, float s, float l)
    {
        float r, g, b;
        if (details::equals(s, 0.0f))
            r = g = b = 1;
        else
        {
            const float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
            const float p = 2.0f * l - q;
            r = details::hue2rgb(p, q, h + 1.0f / 3.0f);
            g = details::hue2rgb(p, q, h);
            b = details::hue2rgb(p, q, h - 1.0f / 3.0f);
        }

        return color::from_rgb(r, g, b);
    }
    // h, s, v must be in 0..1
    static color from_hsv(float h, float s, float v)
    {
        float r, g, b;
        int i = static_cast<int>(h * 6.0f);
        const float f = h * 6.0f - i;
        const float p = v * (1.0f - s);
        const float q = v * (1.0f - f * s);
        const float t = v * (1.0f - (1.0f - f) * s);


        switch (i % 6)
        {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
        }
        return color::from_rgb(r, g, b);
    }
    static color from_string(std::string_view value)
    {
        size_t start = value.front() == '#' ? 1 : 0;
        if ((value.length() - start) % 2 != 0)
            return {};

        const size_t count = (value.length() - start) / 2;
        uint8_t r = 0, g = 0, b = 0, a = 255;
        std::from_chars(
            value.data() + (start * 2),
            value.data() + (start * 2) + 2,
            r,
            16
        );
        if (count > 1)
        {
            ++start;
            std::from_chars(
                value.data() + (start * 2),
                value.data() + (start * 2) + 2,
                g,
                16
            );
        }
        if (count > 2)
        {
            ++start;
            std::from_chars(
                value.data() + (start * 2),
                value.data() + (start * 2) + 2,
                b,
                16
            );
        }
        if (count > 3)
        {
            ++start;
            std::from_chars(
                value.data() + (start * 2),
                value.data() + (start * 2) + 2,
                a,
                16
            );
        }
        return { r, g, b, a };
    }
    bool operator==(const color& rhs) const { return r_ == rhs.r_ && g_ == rhs.g_ && b_ == rhs.b_ && a_ == rhs.a_; }
    bool operator!=(const color& rhs) const { return r_ != rhs.r_ || g_ != rhs.g_ || b_ != rhs.b_ || a_ != rhs.a_; }
    color operator+(const color& rhs) const
    {
        const int r = (r_ + rhs.r_);
        const int g = (g_ + rhs.g_);
        const int b = (b_ + rhs.b_);
        const int a = (a_ + rhs.a_);

        return { (uint8_t)(r < 256 ? (uint8_t)r : 255),
            (uint8_t)(g < 256 ? (uint8_t)g : 255),
            (uint8_t)(b < 256 ? (uint8_t)b : 255),
            (uint8_t)(a < 256 ? (uint8_t)a : 255) };
    }
    color& operator+=(const color& rhs)
    {
        const int r = (r_ + rhs.r_);
        const int g = (g_ + rhs.g_);
        const int b = (b_ + rhs.b_);
        const int a = (a_ + rhs.a_);
        r_ = (uint8_t)(r < 256 ? (uint8_t)r : 255);
        g_ = (uint8_t)(g < 256 ? (uint8_t)g : 255);
        b_ = (uint8_t)(b < 256 ? (uint8_t)b : 255);
        a_ = (uint8_t)(a < 256 ? (uint8_t)a : 255);

        return *this;
    }
    color operator-(const color& rhs) const
    {
        const int r = (r_ - rhs.r_);
        const int g = (g_ - rhs.g_);
        const int b = (b_ - rhs.b_);
        const int a = (a_ - rhs.a_);

        return { (uint8_t)(r >= 0 ? (uint8_t)r : 0),
            (uint8_t)(g >= 0 ? (uint8_t)g : 0),
            (uint8_t)(b >= 0 ? (uint8_t)b : 0),
            (uint8_t)(a >= 0 ? (uint8_t)a : 0) };
    }
    color& operator-=(const color& rhs)
    {
        const int r = (r_ - rhs.r_);
        const int g = (g_ - rhs.g_);
        const int b = (b_ - rhs.b_);
        const int a = (a_ - rhs.a_);
        r_ = (uint8_t)(r >= 0 ? (uint8_t)r : 0);
        g_ = (uint8_t)(g >= 0 ? (uint8_t)g : 0);
        b_ = (uint8_t)(b >= 0 ? (uint8_t)b : 0);
        a_ = (uint8_t)(a >= 0 ? (uint8_t)a : 0);

        return *this;
    }
    color operator*(const color& rhs) const
    {
        const int r = std::clamp((int)(r_ * rhs.r_), 0, 255);
        const int g = std::clamp((int)(g_ * rhs.g_), 0, 255);
        const int b = std::clamp((int)(b_ * rhs.b_), 0, 255);
        const int a = std::clamp((int)(a_ * rhs.a_), 0, 255);

        return { (uint8_t)r,
            (uint8_t)g,
            (uint8_t)b,
            (uint8_t)a };
    }
    color& operator*=(const color& rhs)
    {
        const int r = std::clamp((int)(r_ * rhs.r_), 0, 255);
        const int g = std::clamp((int)(g_ * rhs.g_), 0, 255);
        const int b = std::clamp((int)(b_ * rhs.b_), 0, 255);
        const int a = std::clamp((int)(a_ * rhs.a_), 0, 255);

        r_ = (uint8_t)(r < 256 ? (uint8_t)r : 255);
        g_ = (uint8_t)(g < 256 ? (uint8_t)g : 255);
        b_ = (uint8_t)(b < 256 ? (uint8_t)b : 255);
        a_ = (uint8_t)(a < 256 ? (uint8_t)a : 255);

        return *this;
    }
    color operator*(float rhs) const
    {
        const int r = std::clamp((int)(r_ * rhs), 0, 255);
        const int g = std::clamp((int)(g_ * rhs), 0, 255);
        const int b = std::clamp((int)(b_ * rhs), 0, 255);
        const int a = std::clamp((int)(a_ * rhs), 0, 255);

        return { (uint8_t)r,
            (uint8_t)g,
            (uint8_t)b,
            (uint8_t)a };
    }
    color operator/(const color& rhs) const
    {
        const int r = std::clamp((int)(r_ / rhs.r_), 0, 255);
        const int g = std::clamp((int)(g_ / rhs.g_), 0, 255);
        const int b = std::clamp((int)(b_ / rhs.b_), 0, 255);
        const int a = std::clamp((int)(a_ / rhs.a_), 0, 255);

        return { (uint8_t)r,
            (uint8_t)g,
            (uint8_t)b,
            (uint8_t)a };
    }
    color operator/(float rhs) const
    {
        const int r = std::clamp((int)(r_ / rhs), 0, 255);
        const int g = std::clamp((int)(g_ / rhs), 0, 255);
        const int b = std::clamp((int)(b_ / rhs), 0, 255);
        const int a = std::clamp((int)(a_ / rhs), 0, 255);

        return { (uint8_t)r,
            (uint8_t)g,
            (uint8_t)b,
            (uint8_t)a };
    }
    explicit operator uint32_t() const
    {
        return (a_ << 24u) | (b_ << 16u) | (g_ << 8u) | r_;
    }
    uint32_t to_32() const
    {
        return (a_ << 24u) | (b_ << 16u) | (g_ << 8u) | r_;
    }
    uint32_t to_24() const
    {
        return (b_ << 16u) | (g_ << 8u) | r_;
    }

    std::tuple<float, float, float> to_rgb() const
    {
        return { (float)r_ / 255.0f, (float)g_ / 255.0f, (float)b_ / 255.0f };
    }

    // Returns h, s, l in range 0..1
    std::tuple<float, float, float> to_hsl() const
    {
        const float r = (float)r_ / 255.0f;
        const float g = (float)g_ / 255.0f;
        const float b = (float)b_ / 255.0f;

        const float min = std::min(r, std::min(g, b));
        const float max = std::max(r, std::max(g, b));
        float h = 0.0f, s = 0.0f, l = (max + min) * 0.5f;
        if (details::equals(max, min))
            h = s = 0;
        else
        {
            const float d = max - min;
            s = l > 0.5f ? d / (2.0f - max - min) : d / (max - min);
            if (details::equals(max, r))
                h = (g - b) / d + (g < b ? 6.0f : 0.0f);
            else if (details::equals(max, g))
                h = (b - r) / d + 2.0f;
            else if (details::equals(max, b))
                h = (r - g) / d + 4.0f;

            h /= 6;
        }
        return { h, s, l };
    }
    // Returns h, s, v in range 0..1
    std::tuple<float, float, float> to_hsv() const
    {
        const float r = (float)r_ / 255.0f;
        const float g = (float)g_ / 255.0f;
        const float b = (float)b_ / 255.0f;

        const float min = std::min(r, std::min(g, b));
        const float max = std::max(r, std::max(g, b));
        float h = 0.0f, s = 0.0f, v = max;
        const float d = max - min;
        s = details::equals(max, 0.0f) ? 0.0f : d / max;
        if (details::equals(max, min))
            h = 0;
        else
        {
            if (details::equals(max, r))
                h = (g - b) / d + (g < b ? 6.0f : 0.0f);
            else if (details::equals(max, g))
                h = (b - r) / d + 2.0f;
            else if (details::equals(max, b))
                h = (r - g) / d + 4.0f;

            h /= 6;
        }
        return { h, s, v };
    }

    float hue() const
    {
        const auto [h, s, l] = to_hsl();
        return h;
    }
    void set_hue(float value)
    {
        auto [h, s, l] = to_hsl();
        h = value;
        *this = from_hsl(h, s, l);
    }
    float saturation() const
    {
        const auto [h, s, l] = to_hsl();
        return s;
    }
    void set_saturation(float value)
    {
        auto [h, s, l] = to_hsl();
        s = value;
        *this = from_hsl(h, s, l);
    }
    float lightness() const
    {
        const auto [h, s, l] = to_hsl();
        return s;
    }
    void set_lightness(float value)
    {
        auto [h, s, l] = to_hsl();
        l = value;
        *this = from_hsl(h, s, l);
    }
    float value() const
    {
        const auto [h, s, v] = to_hsv();
        return v;
    }
    void set_value(float value)
    {
        auto [h, s, v] = to_hsv();
        v = value;
        *this = from_hsv(h, s, v);
    }

    void tint(float factor)
    {
        const int r = r_ + static_cast<int>((float)(255 - r_) * factor);
        const int g = g_ + static_cast<int>((float)(255 - g_) * factor);
        const int b = b_ + static_cast<int>((float)(255 - b_) * factor);
        r_ = (uint8_t)std::clamp(r, 0, 255);
        g_ = (uint8_t)std::clamp(g, 0, 255);
        b_ = (uint8_t)std::clamp(b, 0, 255);
    }
    void tint(float factor, float min, float max)
    {
        const float val = (((factor - min) * 100.0f) / (max - min)) * 0.01f;
        tint(val);
    }
    color tinted(float factor) const
    {
        color result = *this;
        result.tint(factor);
        return result;
    }
    color tinted(float factor, float min, float max) const
    {
        color result = *this;
        result.tint(factor, min, max);
        return result;
    }
    void shade(float factor)
    {
        const int r = static_cast<int>((float)r_ * (1.0f - factor));
        const int g = static_cast<int>((float)g_ * (1.0f - factor));
        const int b = static_cast<int>((float)b_ * (1.0f - factor));
        r_ = (uint8_t)std::clamp(r, 0, 255);
        g_ = (uint8_t)std::clamp(g, 0, 255);
        b_ = (uint8_t)std::clamp(b, 0, 255);
    }
    void shade(float factor, float min, float max)
    {
        const float val = (((factor - min) * 100.0f) / (max - min)) * 0.01f;
        shade(val);
    }
    color shaded(float factor) const
    {
        color result = *this;
        result.shade(factor);
        return result;
    }
    color shaded(float factor, float min, float max) const
    {
        color result = *this;
        result.shade(factor, min, max);
        return result;
    }
    void invert(bool invert_alpha = false)
    {
        r_ = 255 - r_;
        g_ = 255 - g_;
        b_ = 255 - b_;
        if (invert_alpha)
            a_ = 255 - a_;
    }
    color lerp(const color& rhs, float t)
    {
        const float invt = 1.0f - t;
        return {
            (uint8_t)((float)r_* invt + (float)rhs.r_ * t),
            (uint8_t)((float)g_* invt + (float)rhs.g_ * t),
            (uint8_t)((float)b_* invt + (float)rhs.b_ * t),
            (uint8_t)((float)a_* invt + (float)rhs.a_ * t)
        };
    }
    // value in range -1.0..1.0
    void scale(float value)
    {
        if (value < 0.0f)
            shade(-value);
        else if (value > 0.0f)
            tint(value);
    }
    void scale(float value, float min, float max, bool inverted = false)
    {
        const float half = (max - min) / 2.0f;
        const float val = (((value - min) * 100.0f) / (max - min)) - half;
        if (!inverted)
        {
            if (val < 0.0f)
                shade(-val, min, max);
            else if (val > 0.0f)
                tint(val, min, max);
        }
        else
            if (val < 0.0f)
                tint(-val, min, max);
            else if (val > 0.0f)
                shade(val, min, max);
    }
    color scaled(float value) const
    {
        color result = *this;
        result.scale(value);
        return result;
    }
    color scaled(float value, float min, float max, bool inverted = false) const
    {
        color result = *this;
        result.scale(value, min, max, inverted);
        return result;
    }
    // Blends rhs over this with rhs' alpha value
    void alpha_blend(const color& rhs)
    {
        const float alpha = (float)rhs.a_ / 255.0f;
        r_ = (uint8_t)(((float)rhs.r_ * alpha) + ((1.0f - alpha) * (float)r_));
        g_ = (uint8_t)(((float)rhs.g_ * alpha) + ((1.0f - alpha) * (float)g_));
        b_ = (uint8_t)(((float)rhs.b_ * alpha) + ((1.0f - alpha) * (float)b_));
    }
    color alpha_blended(const color& rhs) const
    {
        const float alpha = (float)rhs.a_ / 255.0f;
        const uint8_t r = (uint8_t)(((float)rhs.r_ * alpha) + ((1.0f - alpha) * (float)r_));
        const uint8_t g = (uint8_t)(((float)rhs.g_ * alpha) + ((1.0f - alpha) * (float)g_));
        const uint8_t b = (uint8_t)(((float)rhs.b_ * alpha) + ((1.0f - alpha) * (float)b_));
        return { r, g, b, a_ };
    }
    // To Hex string
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::hex;
        ss << std::setw(2) << std::setfill('0') << (int)r_ <<
            std::setw(2) << std::setfill('0') << (int)g_ <<
            std::setw(2) << std::setfill('0') << (int)b_ <<
            std::setw(2) << std::setfill('0') << (int)a_;
        return ss.str();
    }

    uint8_t r_{ 0 };
    uint8_t g_{ 0 };
    uint8_t b_{ 0 };
    uint8_t a_{ 255 };
};

}
