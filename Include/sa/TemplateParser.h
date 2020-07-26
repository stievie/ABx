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

#include <string>
#include <vector>
#include <string_view>

namespace sa {

class TemplateParser;

struct Token
{
    enum class Type
    {
        Invalid,
        String,
        Expression,
        Quote
    };

    Type type;
    size_t start;
    size_t end;
    std::string value;
};

class Template
{
    friend class TemplateParser;
private:
    std::vector<Token> tokens_;
    size_t reserve_{ 256 };
public:
    bool IsEmpty() const { return tokens_.empty(); }
    template<typename Callback>
    std::string ToString(Callback&& callback) const
    {
        std::string result;
        result.reserve(reserve_ + tokens_.size() * 10);
        for (const auto& token : tokens_)
        {
            switch (token.type)
            {
            case Token::Type::Expression:
            case Token::Type::Quote:
                result.append(callback(token));
                break;
            case Token::Type::String:
                result.append(token.value);
                break;
            default:
                break;
            }
        }
        return result;
    }
};

class TemplateParser
{
private:
    size_t index_{ 0 };
    char quote_{ '\0' };
    bool inQuote_{ false };
    std::string_view source_;
    bool Eof() const { return index_ >= source_.size(); }
    char Next() { return source_.at(index_++); }
    char Peek(size_t offset = 1) const
    {
        if (source_.size() > index_ + offset)
            return source_.at(index_ + offset);
        return 0;
    }
    Token GetNextToken()
    {
        Token result;
        result.type = Token::Type::Invalid;
        result.start = index_;
        while (!Eof())
        {
            char c = Next();
            switch (c)
            {
            case '$':
            {
                char c2 = Peek(0);
                if (c2 == '{')
                {
                    if (result.type != Token::Type::Invalid)
                    {
                        result.end = index_ - 1;
                        result.value = std::string(&source_[result.start], result.end - result.start);
                        --index_;
                        return result;
                    }
                    result.type = Token::Type::Expression;
                    result.start += 2;
                    ++index_;
                }
                break;
            }
            case '}':
                if (result.type == Token::Type::Expression)
                {
                    result.end = index_ - 1;
                    result.value = std::string(&source_[result.start], result.end - result.start);
                    return result;
                }
                break;
            case '\0':
                result.end = index_;
                result.value = std::string(&source_[result.start], result.end - result.start);
                return result;
            case '`':
            case '\'':
            case '"':
                if (quotesSupport_)
                {
                    if (inQuote_ && quote_ != c)
                        continue;
                    if (result.type == Token::Type::String)
                    {
                        result.end = index_ - 1;
                        result.value = std::string(&source_[result.start], result.end - result.start);
                        --index_;
                        return result;
                    }
                    result.type = Token::Type::Quote;
                    result.end = index_ - 1;
                    result.value = c;
                    inQuote_ = !inQuote_;
                    quote_ = inQuote_ ? c : '\0';
                    return result;
                }
                // If not supporting quotes fall through default handler
                [[fallthrough]];
            default:
                if (result.type == Token::Type::Invalid)
                    result.type = Token::Type::String;
                break;
            }
        }
        result.end = index_;
        result.value = std::string(&source_[result.start], result.end - result.start);
        return result;
    }
    void Reset()
    {
        index_ = 0;
        quote_ = '\0';
        inQuote_ = false;
    }
public:
    Template Parse(std::string_view source)
    {
        source_ = source;
        Reset();
        Template result;
        result.reserve_ = source_.length();
        while (!Eof())
            result.tokens_.push_back(GetNextToken());
        return result;
    }
    void Append(std::string_view source, Template& tokens)
    {
        source_ = source;
        Reset();
        tokens.reserve_ += source_.length();
        while (!Eof())
            tokens.tokens_.push_back(GetNextToken());
    }
    template<typename Callback>
    static std::string Evaluate(std::string_view source, Callback&& callback)
    {
        TemplateParser parser;
        const Template tokens = parser.Parse(source);
        return tokens.ToString(std::forward<Callback>(callback));
    }
    bool quotesSupport_{ true };
};

}
