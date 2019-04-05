#pragma once

#include <memory>
#include <vector>
#include <random>

namespace Utils {

// https://github.com/m-ochi/aliasmethod
template <typename T>
class WeightedSelector
{
private:
    std::vector<float> weights_;
    std::vector<float> probs_;
    std::vector<size_t> alias_;
    std::vector<T> values_;
    inline size_t GetIndex() const
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<float> r_uni(0.0, 1.0);
        float rand1 = r_uni(gen);
        const size_t count = values_.size();
        size_t k = static_cast<size_t>(count * rand1);
        return rand1 < probs_[k] ? k : alias_[k];
    }
public:
    WeightedSelector() = default;
    // non-copyable
    WeightedSelector(const WeightedSelector&) = delete;
    WeightedSelector& operator=(const WeightedSelector&) = delete;
    ~WeightedSelector() = default;

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

        for (int k = static_cast<int>(count) - 1; k >= 0; k--)
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
    }

    T Get() const
    {
        return values_[GetIndex()];
    }
};

}
