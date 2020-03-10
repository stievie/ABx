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

#include <tinyexpr.h>
#include <vector>
#include <string>
#include <string.h>

class TinyExpr
{
private:
    std::vector<te_variable> variables_;
    std::vector<char*> names_;
    te_expr* expr_{ nullptr };
    void AddFunc(const std::string& name, const void* ptr, int type)
    {
#ifdef _MSC_VER
        char* n = _strdup(name.c_str());
#else
        char* n = strdup(name.c_str());
#endif
        names_.push_back(n);

        te_variable var = { n, ptr, type, nullptr };
        variables_.push_back(std::move(var));
    }
public:
    ~TinyExpr()
    {
        Reset();
    }
    void Reset()
    {
        if (expr_)
        {
            te_free(expr_);
            expr_ = nullptr;
        }
        for (auto& n : names_)
            free(n);
        names_.clear();
        variables_.clear();
    }
    void AddVariable(const std::string& name, double* value)
    {
#ifdef _MSC_VER
        char* n = _strdup(name.c_str());
#else
        char* n = strdup(name.c_str());
#endif
        names_.push_back(n);
        te_variable var = { n, value, TE_VARIABLE, nullptr };
        variables_.push_back(std::move(var));
    }
    void AddFunction(const std::string& name, double(*callback)())
    {
        AddFunc(name, reinterpret_cast<const void*>(callback), TE_FUNCTION0);
    }
    void AddFunction(const std::string& name, double(*callback)(double))
    {
        AddFunc(name, reinterpret_cast<const void*>(callback), TE_FUNCTION1);
    }
    void AddFunction(const std::string& name, double(*callback)(double, double))
    {
        AddFunc(name, reinterpret_cast<const void*>(callback), TE_FUNCTION2);
    }
    void AddFunction(const std::string& name, double(*callback)(double, double, double))
    {
        AddFunc(name, reinterpret_cast<const void*>(callback), TE_FUNCTION3);
    }
    void AddFunction(const std::string& name, double(*callback)(double, double, double, double))
    {
        AddFunc(name, reinterpret_cast<const void*>(callback), TE_FUNCTION4);
    }
    void AddFunction(const std::string& name, double(*callback)(double, double, double, double, double))
    {
        AddFunc(name, reinterpret_cast<const void*>(callback), TE_FUNCTION5);
    }
    void AddFunction(const std::string& name, double(*callback)(double, double, double, double, double, double))
    {
        AddFunc(name, reinterpret_cast<const void*>(callback), TE_FUNCTION6);
    }
    void AddFunction(const std::string& name, double(*callback)(double, double, double, double, double, double, double))
    {
        AddFunc(name, reinterpret_cast<const void*>(callback), TE_FUNCTION7);
    }
    int Compile(const std::string& expr)
    {
        if (expr_)
            te_free(expr_);

        int err;
        expr_ = te_compile(expr.c_str(), variables_.data(), static_cast<int>(variables_.size()), &err);
        return err;
    }
    double Evaluate()
    {
        if (!expr_)
            return 0.0;
        return te_eval(expr_);
    }
};
