/**
 * Copyright 2017-2020 Stefan Ascher
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

#include <memory>
#include <vector>
#include <random>
#include <sa/Assert.h>
#include <sa/Noncopyable.h>

namespace sa {

// https://github.com/m-ochi/aliasmethod
template <typename T>
class WeightedSelector
{
    NON_COPYABLE(WeightedSelector)
private:
    bool initialized_;
    std::vector<float> weights_;
    std::vector<float> probs_;
    std::vector<size_t> alias_;
    std::vector<T> values_;
    size_t GetIndex(float rand1, float rand2) const
    {
        const size_t count = values_.size();
        size_t k = static_cast<size_t>(static_cast<float>(count) * rand1);
        return rand2 < probs_[k] ? k : alias_[k];
    }
public:
    WeightedSelector() :
        initialized_(false)
    { }

    size_t Count() const { return values_.size(); }
    /// Add a value with a weight. After all values have been added call Update() then
    /// call Get() to get a value.
    void Add(const T& value, float weight)
    {
        values_.push_back(value);
        weights_.push_back(weight);
    }

    void Update()
    {
        const size_t count = values_.size();
        std::unique_ptr<float[]> norm_probs = std::make_unique<float[]>(count);
        std::unique_ptr<size_t[]> large_block = std::make_unique<size_t[]>(count);
        std::unique_ptr<size_t[]> small_block = std::make_unique<size_t[]>(count);

        probs_.clear();
        alias_.clear();
        probs_.resize(count);
        alias_.resize(count);

        float sum = 0;
        size_t cur_small_block;
        size_t cur_large_block;
        size_t num_small_block = 0;
        size_t num_large_block = 0;

        for (size_t k = 0; k < count; ++k)
            sum += weights_[k];
        for (size_t k = 0; k < count; ++k)
            norm_probs[k] = weights_[k] * count / sum;

        for (size_t k = count - 1; k != 0; --k)
        {
            if (norm_probs[k] < 1)
                small_block[num_small_block++] = k;
            else
                large_block[num_large_block++] = k;
        }

        while (num_small_block && num_large_block)
        {
            cur_small_block = small_block[--num_small_block];
            cur_large_block = large_block[--num_large_block];
            probs_[cur_small_block] = norm_probs[cur_small_block];
            alias_[cur_small_block] = cur_large_block;
            norm_probs[cur_large_block] = norm_probs[cur_large_block] + norm_probs[cur_small_block] - 1;
            if (norm_probs[cur_large_block] < 1)
                small_block[num_small_block++] = cur_large_block;
            else
                large_block[num_large_block++] = cur_large_block;
        }

        while (num_large_block)
            probs_[large_block[--num_large_block]] = 1.0f;
        while (num_small_block)
            probs_[small_block[--num_small_block]] = 1.0f;
        initialized_ = true;
    }

    bool IsInitialized() const { return initialized_; }

    const T& Get(float rand1, float rand2) const
    {
        ASSERT(initialized_);

        auto i = GetIndex(rand1, rand2);
        ASSERT(Count() > i);
        return values_[i];
    }
};

}
